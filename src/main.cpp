#include <Arduino.h>
#include <esp32_smartdisplay.h>
#include <WiFi.h>
#include "settings.h"
#include "pomodoro.h"
#include "ui.h"
#include "improv_serial.h"
#include "web_portal.h"

static uint32_t lvLastTick = 0;
static uint32_t lastUiRefresh = 0;
static uint32_t ledOffAt = 0;
static bool portalStarted = false;
static bool ntpStarted = false;

void setup() {
  Serial.begin(115200); // Réservé au protocole Improv

  settings_load();
  g_pomodoro.begin();

  smartdisplay_init();
  auto display = lv_display_get_default();
  lv_display_set_rotation(display, LV_DISPLAY_ROTATION_90); // Paysage 320x240
  smartdisplay_lcd_set_backlight(g_settings.brightness / 100.0f);
#ifdef BOARD_HAS_RGB_LED
  smartdisplay_led_set_rgb(false, false, false);
#endif

  ui_init();

  // Reconnexion avec les identifiants stockés en NVS (si déjà provisionné)
  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin();

  improv::begin();
  lvLastTick = millis();
}

static void onWifiConnected() {
  improv::setProvisioned(true);
  if (!ntpStarted) {
    configTzTime(g_settings.tz, "pool.ntp.org", "time.google.com");
    ntpStarted = true;
  }
  if (!portalStarted) {
    web_portal_begin();
    portalStarted = true;
  }
}

void loop() {
  // LVGL
  uint32_t now = millis();
  lv_tick_inc(now - lvLastTick);
  lvLastTick = now;
  lv_timer_handler();

  // Minuteur
  g_pomodoro.tick();

  // Fin de phase : flash LED
  if (g_pomodoro.justFinished()) {
#ifdef BOARD_HAS_RGB_LED
    if (g_settings.ledEnabled) {
      bool focusNext = (g_pomodoro.phase() == PomoPhase::FOCUS);
      // Rouge si retour au focus, vert si pause
      smartdisplay_led_set_rgb(focusNext, !focusNext, false);
      ledOffAt = now + 3000;
    }
#endif
    ui_update();
  }
#ifdef BOARD_HAS_RGB_LED
  if (ledOffAt && now > ledOffAt) {
    smartdisplay_led_set_rgb(false, false, false);
    ledOffAt = 0;
  }
#endif

  // Rafraîchissement de l'affichage (2x/s suffit)
  if (now - lastUiRefresh > 500) {
    lastUiRefresh = now;
    ui_update();
  }

  // Wi-Fi & services
  static wl_status_t lastStatus = WL_IDLE_STATUS;
  wl_status_t st = WiFi.status();
  if (st != lastStatus) {
    lastStatus = st;
    if (st == WL_CONNECTED) onWifiConnected();
    else improv::setProvisioned(false);
  }

  improv::loop();
  if (portalStarted) web_portal_loop();

  delay(2);
}
