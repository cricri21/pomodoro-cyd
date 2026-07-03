#include "presets.h"
#include "settings.h"
#include "pomodoro.h"

// Ordre : nom, focus, pause courte, pause longue, sessions/cycle, couleur, icône
const Preset PRESETS[] = {
  { "Focus",   25, 5, 15, 4, lv_color_hex(0xE8503A), IconKind::TOMATO   },
  { "Emails",  15, 5, 15, 3, lv_color_hex(0x4A90D9), IconKind::ENVELOPE },
  { "Code",    45, 15, 30, 2, lv_color_hex(0x3FBFA8), IconKind::CODE    },
  { "Lecture", 30, 10, 20, 3, lv_color_hex(0xF2A93B), IconKind::BOOK    },
  { "Appels",  20, 5, 15, 4, lv_color_hex(0x9B6BD9), IconKind::CALL     },
};
const uint8_t PRESET_COUNT = sizeof(PRESETS) / sizeof(PRESETS[0]);

void preset_apply(uint8_t index) {
  if (index >= PRESET_COUNT) return;
  const Preset &p = PRESETS[index];
  g_settings.workMin = p.workMin;
  g_settings.shortBreakMin = p.shortBreakMin;
  g_settings.longBreakMin = p.longBreakMin;
  g_settings.sessionsUntilLong = p.sessionsUntilLong;
  g_settings.activePreset = index;
  settings_save();
  g_pomodoro.reload();
}

static void iconBox(lv_obj_t *obj) {
  lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_width(obj, 0, 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
  lv_obj_set_style_pad_all(obj, 0, 0);
}

void preset_draw_icon(lv_obj_t *parent, IconKind icon, lv_color_t color, int size) {
  switch (icon) {
    case IconKind::TOMATO: {
      // Corps rond rouge + petite feuille verte en haut
      lv_obj_t *body = lv_obj_create(parent);
      iconBox(body);
      lv_obj_set_size(body, size, size);
      lv_obj_set_style_radius(body, LV_RADIUS_CIRCLE, 0);
      lv_obj_set_style_bg_opa(body, LV_OPA_COVER, 0);
      lv_obj_set_style_bg_color(body, color, 0);
      lv_obj_center(body);

      lv_obj_t *leaf = lv_obj_create(parent);
      iconBox(leaf);
      lv_obj_set_size(leaf, size / 3, size / 4);
      lv_obj_set_style_radius(leaf, 3, 0);
      lv_obj_set_style_bg_opa(leaf, LV_OPA_COVER, 0);
      lv_obj_set_style_bg_color(leaf, lv_color_hex(0x3FBFA8), 0);
      lv_obj_align(leaf, LV_ALIGN_TOP_MID, 0, -size / 8);
      break;
    }
    case IconKind::ENVELOPE: {
      lv_obj_t *l = lv_label_create(parent);
      lv_label_set_text(l, LV_SYMBOL_ENVELOPE);
      lv_obj_set_style_text_font(l, &lv_font_montserrat_24, 0);
      lv_obj_set_style_text_color(l, color, 0);
      lv_obj_center(l);
      break;
    }
    case IconKind::CODE: {
      lv_obj_t *l = lv_label_create(parent);
      lv_label_set_text(l, "</>");
      lv_obj_set_style_text_font(l, &lv_font_montserrat_20, 0);
      lv_obj_set_style_text_color(l, color, 0);
      lv_obj_center(l);
      break;
    }
    case IconKind::BOOK: {
      // Deux "pages" rectangulaires collées, façon livre ouvert stylisé
      lv_color_t spine = lv_color_darken(color, 60);
      lv_obj_t *left = lv_obj_create(parent);
      iconBox(left);
      lv_obj_set_size(left, size / 2 - 2, (int)(size * 0.8f));
      lv_obj_set_style_radius(left, 3, 0);
      lv_obj_set_style_bg_opa(left, LV_OPA_COVER, 0);
      lv_obj_set_style_bg_color(left, color, 0);
      lv_obj_align(left, LV_ALIGN_CENTER, -size / 4, 0);

      lv_obj_t *right = lv_obj_create(parent);
      iconBox(right);
      lv_obj_set_size(right, size / 2 - 2, (int)(size * 0.8f));
      lv_obj_set_style_radius(right, 3, 0);
      lv_obj_set_style_bg_opa(right, LV_OPA_COVER, 0);
      lv_obj_set_style_bg_color(right, spine, 0);
      lv_obj_align(right, LV_ALIGN_CENTER, size / 4, 0);
      break;
    }
    case IconKind::CALL: {
      lv_obj_t *l = lv_label_create(parent);
      lv_label_set_text(l, LV_SYMBOL_CALL);
      lv_obj_set_style_text_font(l, &lv_font_montserrat_24, 0);
      lv_obj_set_style_text_color(l, color, 0);
      lv_obj_center(l);
      break;
    }
  }
}
