#include "ui.h"
#include "pomodoro.h"
#include "settings.h"
#include <lvgl.h>
#include <WiFi.h>
#include <time.h>

// ------------------------------------------------------------------
// Palette
// ------------------------------------------------------------------
#define COL_BG      lv_color_hex(0x141210)
#define COL_CARD    lv_color_hex(0x1F1B15)
#define COL_TEXT    lv_color_hex(0xF5EFDF)
#define COL_MUTED   lv_color_hex(0xA89F8D)
#define COL_FOCUS   lv_color_hex(0xE8503A)  // tomate
#define COL_SHORT   lv_color_hex(0x3FBFA8)  // sarcelle
#define COL_LONG    lv_color_hex(0xF2C14E)  // jaune CYD

// ------------------------------------------------------------------
// Objets
// ------------------------------------------------------------------
static lv_obj_t *scrMain, *scrSettings;
static lv_obj_t *arc, *lblTime, *lblPhase, *lblClock, *lblWifi;
static lv_obj_t *btnStart, *lblStart;
static lv_obj_t *dots[8];

// Écran réglages
struct SettingRow {
  lv_obj_t *lblValue;
  uint16_t *value;
  uint16_t minV, maxV, step;
};
static SettingRow rows[4];
static uint16_t sessionsTmp; // sessionsUntilLong est uint8_t → copie de travail

static lv_color_t phaseColor(PomoPhase p) {
  switch (p) {
    case PomoPhase::FOCUS:       return COL_FOCUS;
    case PomoPhase::SHORT_BREAK: return COL_SHORT;
    case PomoPhase::LONG_BREAK:  return COL_LONG;
  }
  return COL_FOCUS;
}

static const char *phaseName(PomoPhase p) {
  switch (p) {
    case PomoPhase::FOCUS:       return "FOCUS";
    case PomoPhase::SHORT_BREAK: return "PAUSE COURTE";
    case PomoPhase::LONG_BREAK:  return "PAUSE LONGUE";
  }
  return "";
}

// ------------------------------------------------------------------
// Callbacks
// ------------------------------------------------------------------
static void onStartPause(lv_event_t *) { g_pomodoro.startPause(); ui_update(); }
static void onReset(lv_event_t *)      { g_pomodoro.reset(); ui_update(); }
static void onSkip(lv_event_t *)       { g_pomodoro.skip(); ui_update(); }
static void onOpenSettings(lv_event_t *) { lv_screen_load(scrSettings); }

static void onCloseSettings(lv_event_t *) {
  g_settings.sessionsUntilLong = (uint8_t)sessionsTmp;
  settings_save();
  g_pomodoro.reload();
  lv_screen_load(scrMain);
  ui_update();
}

static void adjustRow(SettingRow *row, int dir) {
  int32_t v = *(row->value) + dir * row->step;
  v = constrain(v, row->minV, row->maxV);
  *(row->value) = (uint16_t)v;
  lv_label_set_text_fmt(row->lblValue, "%d", (int)v);
}
static void onRowMinus(lv_event_t *e) { adjustRow((SettingRow *)lv_event_get_user_data(e), -1); }
static void onRowPlus(lv_event_t *e)  { adjustRow((SettingRow *)lv_event_get_user_data(e), +1); }

// ------------------------------------------------------------------
// Construction : écran principal
// ------------------------------------------------------------------
static lv_obj_t *makeButton(lv_obj_t *parent, const char *txt, lv_color_t bg,
                            lv_event_cb_t cb, int w, int h) {
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_size(btn, w, h);
  lv_obj_set_style_bg_color(btn, bg, 0);
  lv_obj_set_style_radius(btn, 10, 0);
  lv_obj_set_style_shadow_width(btn, 0, 0);
  lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);
  lv_obj_t *l = lv_label_create(btn);
  lv_label_set_text(l, txt);
  lv_obj_set_style_text_color(l, COL_TEXT, 0);
  lv_obj_center(l);
  return btn;
}

static void buildMain() {
  scrMain = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(scrMain, COL_BG, 0);

  // --- Barre du haut ---
  lblWifi = lv_label_create(scrMain);
  lv_obj_set_style_text_color(lblWifi, COL_MUTED, 0);
  lv_obj_set_style_text_font(lblWifi, &lv_font_montserrat_12, 0);
  lv_obj_align(lblWifi, LV_ALIGN_TOP_LEFT, 8, 6);
  lv_label_set_text(lblWifi, LV_SYMBOL_WIFI " ...");

  lblClock = lv_label_create(scrMain);
  lv_obj_set_style_text_color(lblClock, COL_MUTED, 0);
  lv_obj_set_style_text_font(lblClock, &lv_font_montserrat_16, 0);
  lv_obj_align(lblClock, LV_ALIGN_TOP_RIGHT, -10, 4);
  lv_label_set_text(lblClock, "--:--");

  // --- Arc de progression ---
  arc = lv_arc_create(scrMain);
  lv_obj_set_size(arc, 176, 176);
  lv_obj_align(arc, LV_ALIGN_LEFT_MID, 12, 10);
  lv_arc_set_rotation(arc, 270);
  lv_arc_set_bg_angles(arc, 0, 360);
  lv_arc_set_range(arc, 0, 1000);
  lv_arc_set_value(arc, 1000);
  lv_obj_remove_style(arc, nullptr, LV_PART_KNOB);
  lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_width(arc, 10, LV_PART_MAIN);
  lv_obj_set_style_arc_width(arc, 10, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(arc, COL_CARD, LV_PART_MAIN);
  lv_obj_set_style_arc_color(arc, COL_FOCUS, LV_PART_INDICATOR);

  lblTime = lv_label_create(scrMain);
  lv_obj_set_style_text_font(lblTime, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(lblTime, COL_TEXT, 0);
  lv_label_set_text(lblTime, "25:00");
  lv_obj_align_to(lblTime, arc, LV_ALIGN_CENTER, 0, -8);

  lblPhase = lv_label_create(scrMain);
  lv_obj_set_style_text_font(lblPhase, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(lblPhase, COL_FOCUS, 0);
  lv_label_set_text(lblPhase, "FOCUS");
  lv_obj_align_to(lblPhase, arc, LV_ALIGN_CENTER, 0, 28);

  // --- Points de session ---
  for (int i = 0; i < 8; i++) {
    dots[i] = lv_obj_create(scrMain);
    lv_obj_set_size(dots[i], 10, 10);
    lv_obj_set_style_radius(dots[i], LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(dots[i], 0, 0);
    lv_obj_set_style_bg_color(dots[i], COL_CARD, 0);
    lv_obj_remove_flag(dots[i], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(dots[i], LV_OBJ_FLAG_HIDDEN);
  }

  // --- Boutons colonne droite ---
  btnStart = makeButton(scrMain, LV_SYMBOL_PLAY "  Demarrer", COL_FOCUS, onStartPause, 112, 44);
  lv_obj_align(btnStart, LV_ALIGN_TOP_RIGHT, -10, 66);
  lblStart = lv_obj_get_child(btnStart, 0);

  lv_obj_t *b = makeButton(scrMain, LV_SYMBOL_REFRESH "  Reset", COL_CARD, onReset, 112, 36);
  lv_obj_align(b, LV_ALIGN_TOP_RIGHT, -10, 118);

  b = makeButton(scrMain, LV_SYMBOL_NEXT "  Passer", COL_CARD, onSkip, 112, 36);
  lv_obj_align(b, LV_ALIGN_TOP_RIGHT, -10, 160);

  b = makeButton(scrMain, LV_SYMBOL_SETTINGS "  Regler", COL_CARD, onOpenSettings, 112, 36);
  lv_obj_align(b, LV_ALIGN_TOP_RIGHT, -10, 202);
}

// ------------------------------------------------------------------
// Construction : écran réglages
// ------------------------------------------------------------------
static void buildRow(lv_obj_t *parent, int y, const char *name, SettingRow *row) {
  lv_obj_t *lbl = lv_label_create(parent);
  lv_label_set_text(lbl, name);
  lv_obj_set_style_text_color(lbl, COL_TEXT, 0);
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
  lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 12, y + 12);

  // Bouton "-"
  lv_obj_t *minus = lv_button_create(parent);
  lv_obj_set_size(minus, 40, 40);
  lv_obj_set_style_bg_color(minus, COL_CARD, 0);
  lv_obj_set_style_radius(minus, 8, 0);
  lv_obj_align(minus, LV_ALIGN_TOP_RIGHT, -110, y);
  lv_obj_add_event_cb(minus, onRowMinus, LV_EVENT_CLICKED, row);
  lv_obj_t *lm = lv_label_create(minus);
  lv_label_set_text(lm, LV_SYMBOL_MINUS);
  lv_obj_center(lm);

  // Valeur
  row->lblValue = lv_label_create(parent);
  lv_obj_set_style_text_font(row->lblValue, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(row->lblValue, COL_LONG, 0);
  lv_label_set_text_fmt(row->lblValue, "%d", (int)*(row->value));
  lv_obj_align(row->lblValue, LV_ALIGN_TOP_RIGHT, -72, y + 10);

  // Bouton "+"
  lv_obj_t *plus = lv_button_create(parent);
  lv_obj_set_size(plus, 40, 40);
  lv_obj_set_style_bg_color(plus, COL_CARD, 0);
  lv_obj_set_style_radius(plus, 8, 0);
  lv_obj_align(plus, LV_ALIGN_TOP_RIGHT, -12, y);
  lv_obj_add_event_cb(plus, onRowPlus, LV_EVENT_CLICKED, row);
  lv_obj_t *lp = lv_label_create(plus);
  lv_label_set_text(lp, LV_SYMBOL_PLUS);
  lv_obj_center(lp);
}

static void buildSettings() {
  scrSettings = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(scrSettings, COL_BG, 0);

  lv_obj_t *title = lv_label_create(scrSettings);
  lv_label_set_text(title, "Reglages (minutes)");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(title, COL_LONG, 0);
  lv_obj_align(title, LV_ALIGN_TOP_LEFT, 12, 8);

  sessionsTmp = g_settings.sessionsUntilLong;
  rows[0] = { nullptr, &g_settings.workMin, 1, 120, 1 };
  rows[1] = { nullptr, &g_settings.shortBreakMin, 1, 60, 1 };
  rows[2] = { nullptr, &g_settings.longBreakMin, 1, 90, 1 };
  rows[3] = { nullptr, &sessionsTmp, 2, 8, 1 };

  buildRow(scrSettings, 34,  "Focus", &rows[0]);
  buildRow(scrSettings, 80,  "Pause courte", &rows[1]);
  buildRow(scrSettings, 126, "Pause longue", &rows[2]);
  buildRow(scrSettings, 172, "Sessions / cycle", &rows[3]);

  lv_obj_t *back = makeButton(scrSettings, LV_SYMBOL_OK "  Valider", COL_SHORT, onCloseSettings, 296, 22);
  lv_obj_set_style_bg_color(back, COL_SHORT, 0);
  lv_obj_align(back, LV_ALIGN_BOTTOM_MID, 0, -4);
}

// ------------------------------------------------------------------
// API publique
// ------------------------------------------------------------------
void ui_init() {
  buildMain();
  buildSettings();
  lv_screen_load(scrMain);
  ui_update();
}

void ui_apply_settings() {
  for (auto &r : rows)
    if (r.lblValue) lv_label_set_text_fmt(r.lblValue, "%d", (int)*(r.value));
  sessionsTmp = g_settings.sessionsUntilLong;
}

void ui_update() {
  // Temps restant
  uint32_t s = (g_pomodoro.remainingMs() + 999) / 1000;
  lv_label_set_text_fmt(lblTime, "%02u:%02u", (unsigned)(s / 60), (unsigned)(s % 60));
  lv_obj_align_to(lblTime, arc, LV_ALIGN_CENTER, 0, -8);

  // Arc
  uint32_t total = g_pomodoro.totalMs();
  int32_t v = total ? (int32_t)((uint64_t)g_pomodoro.remainingMs() * 1000 / total) : 0;
  lv_arc_set_value(arc, v);
  lv_color_t pc = phaseColor(g_pomodoro.phase());
  lv_obj_set_style_arc_color(arc, pc, LV_PART_INDICATOR);

  // Phase
  lv_label_set_text(lblPhase, phaseName(g_pomodoro.phase()));
  lv_obj_set_style_text_color(lblPhase, pc, 0);
  lv_obj_align_to(lblPhase, arc, LV_ALIGN_CENTER, 0, 28);

  // Bouton principal
  if (g_pomodoro.state() == PomoState::RUNNING)
    lv_label_set_text(lblStart, LV_SYMBOL_PAUSE "  Pause");
  else if (g_pomodoro.state() == PomoState::PAUSED)
    lv_label_set_text(lblStart, LV_SYMBOL_PLAY "  Reprendre");
  else
    lv_label_set_text(lblStart, LV_SYMBOL_PLAY "  Demarrer");
  lv_obj_set_style_bg_color(btnStart, pc, 0);

  // Points de session
  uint8_t n = g_settings.sessionsUntilLong;
  int spacing = 18;
  int x0 = -10 - 112 + (112 - (n - 1) * spacing) / 2 - 5;
  for (int i = 0; i < 8; i++) {
    if (i < n) {
      lv_obj_remove_flag(dots[i], LV_OBJ_FLAG_HIDDEN);
      lv_obj_align(dots[i], LV_ALIGN_TOP_RIGHT, x0 + i * spacing, 40);
      lv_obj_set_style_bg_color(dots[i],
        (i < g_pomodoro.completedInCycle()) ? COL_FOCUS : COL_CARD, 0);
    } else {
      lv_obj_add_flag(dots[i], LV_OBJ_FLAG_HIDDEN);
    }
  }

  // Horloge (NTP)
  time_t now = time(nullptr);
  if (now > 1600000000) {
    struct tm tmInfo;
    localtime_r(&now, &tmInfo);
    lv_label_set_text_fmt(lblClock, "%02d:%02d", tmInfo.tm_hour, tmInfo.tm_min);
  }

  // Wi-Fi
  if (WiFi.status() == WL_CONNECTED)
    lv_label_set_text_fmt(lblWifi, LV_SYMBOL_WIFI " %s", WiFi.localIP().toString().c_str());
  else
    lv_label_set_text(lblWifi, LV_SYMBOL_WIFI " non connecte");
}
