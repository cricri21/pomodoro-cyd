#include "settings.h"
#include <Preferences.h>

PomodoroSettings g_settings;
static Preferences prefs;

void settings_load() {
  prefs.begin("pomodoro", true);
  g_settings.workMin = prefs.getUShort("work", 25);
  g_settings.shortBreakMin = prefs.getUShort("sbrk", 5);
  g_settings.longBreakMin = prefs.getUShort("lbrk", 15);
  g_settings.sessionsUntilLong = prefs.getUChar("sess", 4);
  g_settings.autoStart = prefs.getBool("auto", false);
  g_settings.brightness = prefs.getUChar("bright", 100);
  g_settings.ledEnabled = prefs.getBool("led", true);
  String tz = prefs.getString("tz", "CET-1CEST,M3.5.0,M10.5.0/3");
  strlcpy(g_settings.tz, tz.c_str(), sizeof(g_settings.tz));
  g_settings.activePreset = prefs.getUChar("preset", 0);
  prefs.end();

  // Garde-fous
  g_settings.workMin = constrain(g_settings.workMin, 1, 120);
  g_settings.shortBreakMin = constrain(g_settings.shortBreakMin, 1, 60);
  g_settings.longBreakMin = constrain(g_settings.longBreakMin, 1, 90);
  g_settings.sessionsUntilLong = constrain(g_settings.sessionsUntilLong, 2, 8);
  g_settings.brightness = constrain(g_settings.brightness, 10, 100);
}

void settings_save() {
  prefs.begin("pomodoro", false);
  prefs.putUShort("work", g_settings.workMin);
  prefs.putUShort("sbrk", g_settings.shortBreakMin);
  prefs.putUShort("lbrk", g_settings.longBreakMin);
  prefs.putUChar("sess", g_settings.sessionsUntilLong);
  prefs.putBool("auto", g_settings.autoStart);
  prefs.putUChar("bright", g_settings.brightness);
  prefs.putBool("led", g_settings.ledEnabled);
  prefs.putString("tz", g_settings.tz);
  prefs.putUChar("preset", g_settings.activePreset);
  prefs.end();
}
