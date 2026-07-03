#pragma once
#include <Arduino.h>
#include <vector>

// Implémentation du protocole Improv Wi-Fi Serial (https://www.improv-wifi.com/serial/)
// Permet la configuration du Wi-Fi directement depuis le navigateur via ESP Web Tools.

namespace improv {

enum State : uint8_t {
  STATE_READY = 0x02,        // Autorisé, en attente d'identifiants
  STATE_PROVISIONING = 0x03, // Connexion en cours
  STATE_PROVISIONED = 0x04,  // Connecté
};

enum Error : uint8_t {
  ERROR_NONE = 0x00,
  ERROR_INVALID_RPC = 0x01,
  ERROR_UNKNOWN_RPC = 0x02,
  ERROR_UNABLE_TO_CONNECT = 0x03,
  ERROR_UNKNOWN = 0xFF,
};

enum PacketType : uint8_t {
  TYPE_CURRENT_STATE = 0x01,
  TYPE_ERROR_STATE = 0x02,
  TYPE_RPC_COMMAND = 0x03,
  TYPE_RPC_RESPONSE = 0x04,
};

enum Command : uint8_t {
  CMD_WIFI_SETTINGS = 0x01,
  CMD_GET_CURRENT_STATE = 0x02,
  CMD_GET_DEVICE_INFO = 0x03,
  CMD_GET_WIFI_NETWORKS = 0x04,
};

void begin();
void loop();                 // À appeler dans loop() : parse le flux série
void setProvisioned(bool ok);// À appeler quand le Wi-Fi (re)connecte

} // namespace improv
