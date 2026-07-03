#include "web_portal.h"
#include "settings.h"
#include "pomodoro.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <esp32_smartdisplay.h>

static WebServer server(80);
extern void ui_apply_settings(); // défini dans ui.cpp

// ---------------------------------------------------------------
// Page HTML embarquée (légère, sans dépendance externe)
// ---------------------------------------------------------------
static const char PAGE_HEADER[] PROGMEM = R"html(<!DOCTYPE html>
<html lang="fr"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Pomodoro CYD — Configuration</title>
<style>
:root{--bg:#141210;--card:#1f1b15;--yellow:#f2c14e;--tomato:#e8503a;
--teal:#3fbfa8;--text:#f5efdf;--muted:#a89f8d}
*{box-sizing:border-box}
body{margin:0;font-family:system-ui,sans-serif;background:var(--bg);
color:var(--text);padding:16px;max-width:560px;margin-inline:auto}
h1{font-size:1.4rem;color:var(--yellow);margin:8px 0 4px}
h1 span{color:var(--tomato)}
p.sub{color:var(--muted);margin:0 0 20px;font-size:.9rem}
.card{background:var(--card);border:1px solid #2e2820;border-radius:12px;
padding:16px;margin-bottom:16px}
.card h2{font-size:1rem;margin:0 0 12px;color:var(--yellow)}
label{display:flex;justify-content:space-between;align-items:center;
margin:10px 0;font-size:.95rem;gap:12px}
input[type=number],input[type=text],input[type=password],select{
background:#141210;border:1px solid #3a332a;color:var(--text);
border-radius:8px;padding:8px 10px;width:110px;font-size:1rem}
input[type=text],input[type=password]{width:100%}
input[type=checkbox]{width:20px;height:20px;accent-color:var(--tomato)}
input[type=range]{width:150px;accent-color:var(--yellow)}
button{background:var(--tomato);color:#fff;border:0;border-radius:8px;
padding:12px 18px;font-size:1rem;cursor:pointer;width:100%;font-weight:600}
button.secondary{background:transparent;border:1px solid var(--teal);
color:var(--teal);margin-top:8px}
.status{font-size:.85rem;color:var(--muted);margin-top:12px}
.ok{color:var(--teal)}
</style></head><body>
<h1>Pomodoro <span>CYD</span></h1>
<p class="sub">Configuration — les changements s'appliquent immédiatement à l'écran.</p>
)html";

static const char PAGE_FOOTER[] PROGMEM = R"html(
<div class="card"><h2>Wi-Fi</h2>
<form method="POST" action="/wifi">
<label>Réseau (SSID)<input type="text" name="ssid" required></label>
<label>Mot de passe<input type="password" name="pass"></label>
<button class="secondary" type="submit">Changer de réseau Wi-Fi</button>
</form>
<p class="status">Le CYD redémarrera la connexion. Si l'adresse change,
retrouvez-la sur l'écran de l'appareil ou via pomodoro.local.</p>
</div>
<script>
document.getElementById('br').addEventListener('input',e=>{
document.getElementById('brv').textContent=e.target.value+'%';});
</script>
</body></html>)html";

static String settingsForm() {
  String h = "";
  h += "<div class='card'><h2>Minuteur</h2><form method='POST' action='/save'>";
  h += "<label>Focus (min)<input type='number' name='work' min='1' max='120' value='" + String(g_settings.workMin) + "'></label>";
  h += "<label>Pause courte (min)<input type='number' name='sbrk' min='1' max='60' value='" + String(g_settings.shortBreakMin) + "'></label>";
  h += "<label>Pause longue (min)<input type='number' name='lbrk' min='1' max='90' value='" + String(g_settings.longBreakMin) + "'></label>";
  h += "<label>Sessions avant pause longue<input type='number' name='sess' min='2' max='8' value='" + String(g_settings.sessionsUntilLong) + "'></label>";
  h += "<label>Enchaîner automatiquement<input type='checkbox' name='auto' " + String(g_settings.autoStart ? "checked" : "") + "></label>";
  h += "</div><div class='card'><h2>Écran &amp; LED</h2>";
  h += "<label>Luminosité <span id='brv'>" + String(g_settings.brightness) + "%</span>";
  h += "<input type='range' id='br' name='bright' min='10' max='100' value='" + String(g_settings.brightness) + "'></label>";
  h += "<label>Flash LED en fin de phase<input type='checkbox' name='led' " + String(g_settings.ledEnabled ? "checked" : "") + "></label>";
  h += "<label>Fuseau horaire (POSIX)<input type='text' name='tz' value='" + String(g_settings.tz) + "'></label>";
  h += "<button type='submit'>Enregistrer</button>";
  h += "<p class='status'>Sessions focus terminées : <span class='ok'>" + String(g_pomodoro.completedToday()) + "</span>";
  h += " — IP : " + WiFi.localIP().toString() + "</p>";
  h += "</form></div>";
  return h;
}

static void handleRoot() {
  String page = FPSTR(PAGE_HEADER);
  page += settingsForm();
  page += FPSTR(PAGE_FOOTER);
  server.send(200, "text/html", page);
}

static void handleSave() {
  if (server.hasArg("work")) g_settings.workMin = constrain(server.arg("work").toInt(), 1, 120);
  if (server.hasArg("sbrk")) g_settings.shortBreakMin = constrain(server.arg("sbrk").toInt(), 1, 60);
  if (server.hasArg("lbrk")) g_settings.longBreakMin = constrain(server.arg("lbrk").toInt(), 1, 90);
  if (server.hasArg("sess")) g_settings.sessionsUntilLong = constrain(server.arg("sess").toInt(), 2, 8);
  g_settings.autoStart = server.hasArg("auto");
  g_settings.ledEnabled = server.hasArg("led");
  if (server.hasArg("bright")) g_settings.brightness = constrain(server.arg("bright").toInt(), 10, 100);
  if (server.hasArg("tz")) {
    strlcpy(g_settings.tz, server.arg("tz").c_str(), sizeof(g_settings.tz));
    configTzTime(g_settings.tz, "pool.ntp.org", "time.google.com");
  }
  settings_save();
  smartdisplay_lcd_set_backlight(g_settings.brightness / 100.0f);
  g_pomodoro.reload();
  ui_apply_settings();
  server.sendHeader("Location", "/");
  server.send(303);
}

static void handleWifi() {
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");
  server.send(200, "text/html",
    "<meta charset='utf-8'><body style='font-family:sans-serif;background:#141210;color:#f5efdf;padding:24px'>"
    "<h2>Connexion à « " + ssid + " »…</h2>"
    "<p>Le Pomodoro change de réseau. Consultez l'écran du CYD pour la nouvelle adresse IP, "
    "ou réessayez <a style='color:#f2c14e' href='http://pomodoro.local'>pomodoro.local</a> dans quelques secondes.</p></body>");
  delay(300);
  WiFi.persistent(true);
  WiFi.begin(ssid.c_str(), pass.c_str());
}

static void handleState() {
  // Petit endpoint JSON, pratique pour Home Assistant (RESTful sensor)
  const char *phase = g_pomodoro.phase() == PomoPhase::FOCUS ? "focus"
                    : g_pomodoro.phase() == PomoPhase::SHORT_BREAK ? "short_break" : "long_break";
  const char *state = g_pomodoro.state() == PomoState::RUNNING ? "running"
                    : g_pomodoro.state() == PomoState::PAUSED ? "paused" : "idle";
  String json = "{\"phase\":\"" + String(phase) + "\",\"state\":\"" + String(state) +
                "\",\"remaining_s\":" + String(g_pomodoro.remainingMs() / 1000) +
                ",\"completed\":" + String(g_pomodoro.completedToday()) + "}";
  server.send(200, "application/json", json);
}

void web_portal_begin() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/wifi", HTTP_POST, handleWifi);
  server.on("/api/state", HTTP_GET, handleState);
  server.begin();
  MDNS.begin("pomodoro");
  MDNS.addService("http", "tcp", 80);
}

void web_portal_loop() {
  server.handleClient();
}
