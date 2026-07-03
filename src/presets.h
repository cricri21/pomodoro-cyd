#pragma once
#include <lvgl.h>
#include <stdint.h>

// Types d'icônes "classiques" dessinées directement en LVGL (pas d'image externe)
enum class IconKind : uint8_t { TOMATO, ENVELOPE, CODE, BOOK, CALL };

struct Preset {
  const char *name;
  uint16_t workMin;
  uint16_t shortBreakMin;
  uint16_t longBreakMin;
  uint8_t sessionsUntilLong;
  lv_color_t color;
  IconKind icon;
};

extern const Preset PRESETS[];
extern const uint8_t PRESET_COUNT;

// Applique un preset aux réglages actifs, sauvegarde en NVS et recharge le minuteur (si IDLE)
void preset_apply(uint8_t index);

// Dessine une icône colorée classique dans un conteneur LVGL déjà dimensionné
void preset_draw_icon(lv_obj_t *parent, IconKind icon, lv_color_t color, int size);
