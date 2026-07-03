#include "improv_serial.h"
#include <WiFi.h>

namespace improv {

static const char HEADER[6] = {'I', 'M', 'P', 'R', 'O', 'V'};
static const uint8_t VERSION = 1;

static uint8_t currentState = STATE_READY;

// ---------- Émission ----------

static void sendPacket(uint8_t type, const uint8_t *data, uint8_t len) {
  uint8_t buf[300];
  size_t pos = 0;
  memcpy(buf, HEADER, 6); pos += 6;
  buf[pos++] = VERSION;
  buf[pos++] = type;
  buf[pos++] = len;
  memcpy(buf + pos, data, len); pos += len;
  uint8_t checksum = 0;
  for (size_t i = 0; i < pos; i++) checksum += buf[i];
  buf[pos++] = checksum;
  Serial.write(buf, pos);
  Serial.write('\n');
}

static void sendState(uint8_t state) {
  currentState = state;
  sendPacket(TYPE_CURRENT_STATE, &state, 1);
}

static void sendError(uint8_t error) {
  sendPacket(TYPE_ERROR_STATE, &error, 1);
}

// Réponse RPC : [cmd][len][chaînes préfixées par leur longueur]
static void sendRpcResponse(uint8_t cmd, const std::vector<String> &strings) {
  uint8_t data[280];
  size_t pos = 2;
  for (auto &s : strings) {
    uint8_t l = (uint8_t)min((size_t)255, s.length());
    if (pos + 1 + l > sizeof(data)) break;
    data[pos++] = l;
    memcpy(data + pos, s.c_str(), l);
    pos += l;
  }
  data[0] = cmd;
  data[1] = (uint8_t)(pos - 2);
  sendPacket(TYPE_RPC_RESPONSE, data, (uint8_t)pos);
}

static String deviceUrl() {
  return String("http://") + WiFi.localIP().toString() + "/";
}

// ---------- Commandes ----------

static void handleWifiSettings(const uint8_t *data, uint8_t len) {
  // data = [ssid_len][ssid...][pass_len][pass...]
  if (len < 2) { sendError(ERROR_INVALID_RPC); return; }
  uint8_t ssidLen = data[0];
  if (1 + ssidLen + 1 > len) { sendError(ERROR_INVALID_RPC); return; }
  String ssid, pass;
  ssid.reserve(ssidLen);
  for (uint8_t i = 0; i < ssidLen; i++) ssid += (char)data[1 + i];
  uint8_t passLen = data[1 + ssidLen];
  for (uint8_t i = 0; i < passLen && (2 + ssidLen + i) < len; i++)
    pass += (char)data[2 + ssidLen + i];

  sendState(STATE_PROVISIONING);

  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    sendState(STATE_PROVISIONED);
    std::vector<String> urls = { deviceUrl() };
    sendRpcResponse(CMD_WIFI_SETTINGS, urls);
  } else {
    sendState(STATE_READY);
    sendError(ERROR_UNABLE_TO_CONNECT);
  }
}

static void handleGetDeviceInfo() {
  std::vector<String> info = {
    FIRMWARE_NAME,
    FIRMWARE_VERSION,
    "ESP32",
    "Pomodoro CYD"
  };
  sendRpcResponse(CMD_GET_DEVICE_INFO, info);
}

static void handleGetWifiNetworks() {
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n && i < 20; i++) {
    std::vector<String> net = {
      WiFi.SSID(i),
      String(WiFi.RSSI(i)),
      (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? String("NO") : String("YES")
    };
    sendRpcResponse(CMD_GET_WIFI_NETWORKS, net);
    delay(1);
  }
  WiFi.scanDelete();
  // Paquet vide = fin de liste
  std::vector<String> empty;
  sendRpcResponse(CMD_GET_WIFI_NETWORKS, empty);
}

static void handleRpc(const uint8_t *payload, uint8_t len) {
  if (len < 2) { sendError(ERROR_INVALID_RPC); return; }
  uint8_t cmd = payload[0];
  uint8_t dataLen = payload[1];
  const uint8_t *data = payload + 2;
  if (dataLen + 2 > len) { sendError(ERROR_INVALID_RPC); return; }

  switch (cmd) {
    case CMD_WIFI_SETTINGS:
      handleWifiSettings(data, dataLen);
      break;
    case CMD_GET_CURRENT_STATE: {
      sendState(currentState);
      if (currentState == STATE_PROVISIONED) {
        std::vector<String> urls = { deviceUrl() };
        sendRpcResponse(CMD_GET_CURRENT_STATE, urls);
      }
      break;
    }
    case CMD_GET_DEVICE_INFO:
      handleGetDeviceInfo();
      break;
    case CMD_GET_WIFI_NETWORKS:
      handleGetWifiNetworks();
      break;
    default:
      sendError(ERROR_UNKNOWN_RPC);
  }
}

// ---------- Réception ----------

static uint8_t rxBuf[300];
static size_t rxPos = 0;

void begin() {
  currentState = (WiFi.status() == WL_CONNECTED) ? STATE_PROVISIONED : STATE_READY;
}

void setProvisioned(bool ok) {
  currentState = ok ? STATE_PROVISIONED : STATE_READY;
}

void loop() {
  while (Serial.available()) {
    uint8_t b = (uint8_t)Serial.read();

    if (rxPos < 6) {
      // Synchronisation sur l'entête "IMPROV"
      if (b == (uint8_t)HEADER[rxPos]) {
        rxBuf[rxPos++] = b;
      } else {
        rxPos = (b == (uint8_t)HEADER[0]) ? 1 : 0;
        if (rxPos == 1) rxBuf[0] = b;
      }
      continue;
    }

    rxBuf[rxPos++] = b;

    if (rxPos >= 9) {
      uint8_t len = rxBuf[8];
      size_t expected = 9 + len + 1; // header+ver+type+len + data + checksum
      if (rxPos == expected) {
        uint8_t checksum = 0;
        for (size_t i = 0; i < expected - 1; i++) checksum += rxBuf[i];
        if (checksum == rxBuf[expected - 1] && rxBuf[6] == VERSION &&
            rxBuf[7] == TYPE_RPC_COMMAND) {
          handleRpc(rxBuf + 9, len);
        }
        rxPos = 0;
      } else if (rxPos >= sizeof(rxBuf)) {
        rxPos = 0; // Sécurité débordement
      }
    }
  }
}

} // namespace improv
