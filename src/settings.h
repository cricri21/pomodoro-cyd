#pragma once
#include <Arduino.h>

struct PomodoroSettings {
  uint16_t workMin = 25;         // Durée focus (minutes)
  uint16_t shortBreakMin = 5;    // Pause courte
  uint16_t longBreakMin = 15;    // Pause longue
  uint8_t sessionsUntilLong = 4; // Sessions avant pause longue
  bool autoStart = false;        // Enchaîner automatiquement les phases
  uint8_t brightness = 100;      // Rétroéclairage en %
  bool ledEnabled = true;        // Flash LED RGB en fin de phase
  char tz[64] = "CET-1CEST,M3.5.0,M10.5.0/3"; // Europe/Bruxelles
  uint8_t activePreset = 0;      // Index du dernier preset (type de session) choisi
};

extern PomodoroSettings g_settings;

void settings_load();
void settings_save();
