# Pomodoro CYD 🍅

Minuteur Pomodoro tactile pour ESP32 **Cheap Yellow Display** (ESP32-2432S028R, écran 2.8" ILI9341 + tactile XPT2046), flashable **directement depuis le navigateur**.

- Interface **LVGL 9** : arc de progression, durées réglables au tactile (+/−), points de cycle
- **Improv Wi-Fi Serial** : configuration du Wi-Fi depuis le navigateur juste après le flash
- **Portail web local** (`http://pomodoro.local`) : durées, luminosité, LED, fuseau horaire, changement de Wi-Fi
- Horloge **NTP**, endpoint **`/api/state`** (JSON) pour Home Assistant
- Site vitrine + **web flasher** (ESP Web Tools) déployé automatiquement sur GitHub Pages

## Structure

```
├── platformio.ini            # Envs : esp32-2432S028R (+v2, v3)
├── boards/                   # Définitions de carte (rzeldent/platformio-espressif32-sunton)
├── include/lv_conf.h         # Config LVGL
├── src/
│   ├── main.cpp              # Boucle principale
│   ├── pomodoro.{h,cpp}      # Machine à états du minuteur
│   ├── ui.{h,cpp}            # Écrans LVGL (minuteur + réglages)
│   ├── improv_serial.{h,cpp} # Protocole Improv Wi-Fi Serial
│   ├── web_portal.{h,cpp}    # Portail web de configuration + API
│   └── settings.{h,cpp}      # Persistance NVS
├── web/                      # Site + flasher (déployé sur GitHub Pages)
│   ├── index.html
│   ├── manifest.json         # Manifest ESP Web Tools
│   └── firmware/             # Binaires fusionnés (générés par la CI)
└── .github/workflows/build.yml
```

## Mise en route

### 1. Compiler en local (optionnel)

```bash
pip install platformio
pio run -e esp32-2432S028R          # compilation
pio run -e esp32-2432S028R -t upload  # flash direct par USB
```

### 2. Publier le site avec le flasher

1. Créez un dépôt GitHub et poussez ce projet sur la branche `main`.
2. Dans **Settings → Pages**, choisissez **Source : GitHub Actions**.
3. Le workflow compile le firmware, fusionne le binaire (bootloader + partitions + app)
   et déploie `web/` sur Pages. Votre flasher est en ligne à
   `https://<user>.github.io/<repo>/`.

> Web Serial exige HTTPS et Chrome/Edge — GitHub Pages fournit le HTTPS d'office.

### 3. Flasher et configurer

1. Ouvrez le site, branchez le CYD (câble USB **data**), cliquez sur **Flasher**.
2. À la fin du flash, la fenêtre propose de configurer le **Wi-Fi** (Improv).
3. Cliquez sur le lien affiché (ou `http://pomodoro.local`) pour ouvrir le portail
   de configuration.

## Variantes de carte

| Modèle | Env PlatformIO | Binaire CI |
|---|---|---|
| CYD classique (1 micro-USB) | `esp32-2432S028R` | `pomodoro-cyd.bin` |
| CYD double USB (micro + USB-C) | `esp32-2432S028Rv2` | `pomodoro-cyd-v2.bin` |
| CYD révision 3 | `esp32-2432S028Rv3` | — (à ajouter au workflow si besoin) |

Pour proposer la v2 sur le site, créez un second `manifest-v2.json` pointant vers
`firmware/pomodoro-cyd-v2.bin` et un second bouton `esp-web-install-button`.

## API pour Home Assistant

```yaml
rest:
  - resource: http://pomodoro.local/api/state
    scan_interval: 10
    sensor:
      - name: "Pomodoro phase"
        value_template: "{{ value_json.phase }}"
      - name: "Pomodoro restant"
        value_template: "{{ value_json.remaining_s }}"
        unit_of_measurement: "s"
```

## Crédits

- [esp32-smartdisplay](https://github.com/rzeldent/esp32-smartdisplay) (drivers CYD + LVGL)
- [ESP Web Tools](https://esphome.github.io/esp-web-tools/) (flash navigateur)
- [Improv Wi-Fi](https://www.improv-wifi.com/) (provisioning série)
- Inspiré par [Atmos Weather Station](https://leroyd.com/atmos/)
