#pragma once

void ui_init();            // Construit les écrans LVGL
void ui_update();          // Rafraîchit arc, temps, horloge, Wi-Fi (appel périodique)
void ui_apply_settings();  // Ré-applique les réglages (après sauvegarde web)
