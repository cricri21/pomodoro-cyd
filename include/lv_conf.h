/**
 * lv_conf.h — configuration LVGL 9 pour Pomodoro CYD
 * Les options absentes prennent les valeurs par défaut de LVGL.
 */
#if 1 /* Enable content */
#ifndef LV_CONF_H
#define LV_CONF_H

/* Couleur RGB565 (ILI9341) */
#define LV_COLOR_DEPTH 16

/* Mémoire interne LVGL */
#define LV_MEM_SIZE (48 * 1024U)

/* Rafraîchissement */
#define LV_DEF_REFR_PERIOD 33

/* Polices utilisées par l'interface */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_48 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Widgets nécessaires (le reste garde son défaut) */
#define LV_USE_ARC    1
#define LV_USE_BUTTON 1
#define LV_USE_LABEL  1
#define LV_USE_SLIDER 1
#define LV_USE_SWITCH 1

/* Pas de logs LVGL (Serial réservé à Improv) */
#define LV_USE_LOG 0

#endif /* LV_CONF_H */
#endif /* Enable content */
