#pragma once
#include <Arduino.h>

// Portail web de configuration accessible sur http://pomodoro.local
// (ou l'IP du CYD). Permet de régler durées, luminosité, LED, Wi-Fi.

void web_portal_begin();
void web_portal_loop();
