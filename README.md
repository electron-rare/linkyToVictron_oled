# linkyToVictron OLED

Firmware ESP8266 (NodeMCU v2) qui lit la teleinfo d'un compteur Linky triphase et affiche les donnees en temps reel sur un ecran OLED SSD1306.

## Cablage

| Composant       | Broche |
|-----------------|--------|
| OLED SDA        | D1     |
| OLED SCL        | D2     |
| Teleinfo RX     | D7     |
| OLED I2C addr   | 0x3C   |

## Fonctionnalites

- **Auto-detection TIC** : bascule automatiquement entre mode Standard (9600 bauds) et Historique (1200 bauds)
- **Triphase** : courant, tension et puissance par phase
- **Injection solaire** : gestion des puissances negatives (SINSTI, EAIT)
- **Index** : HC, HP, injection, total en kWh
- **Affichage OLED** : 3 pages en alternance (temps reel / index energie / reseau)
- **MQTT Victron** : publication au format `Ac/*` compatible `dbus-mqtt-grid`

## Structure

```text
src/
  Config.h            -- Configuration par defaut (sans secrets)
  ConfigUser.example.h -- Exemple de config locale
  main.cpp            -- Setup/loop, instanciation des objets
  TeleinfoData.h      -- Struct de donnees teleinfo
  TeleinfoReader.h/.cpp -- Lecture serie + parsing des trames TIC
  WifiManager.h/.cpp  -- Connexion Wi-Fi
  MqttPublisher.h/.cpp -- Publication MQTT
  OledDisplay.h/.cpp  -- Affichage OLED SSD1306
platformio.ini        -- Configuration PlatformIO
```

## Configuration locale

Creer une configuration locale non versionnee :

```bash
cp src/ConfigUser.example.h src/ConfigUser.h
```

Puis editer `src/ConfigUser.h` avec vos identifiants Wi-Fi et MQTT.

## Compilation

```bash
pio run
```

## Upload

```bash
pio run --target upload
```

## Dependances

- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [LibTeleinfo](https://github.com/hallard/LibTeleinfo)
- [PubSubClient](https://github.com/knolleary/pubsubclient)























<!-- CHANTIER:AUDIT START -->
## Audit & Execution Plan (2026-03-10)

### Snapshot
- Priority: `P2`
- Tech profile: `embedded`
- Workflows: `yes`
- Tests: `yes`
- Debt markers: `0`
- Source files: `12`

### Corrections Prioritaires
- [ ] Vérifier target PlatformIO et budget mémoire
- [ ] Ajouter/fiabiliser les commandes de vérification automatiques.
- [ ] Clore les points bloquants avant optimisation avancée.

### Optimisation
- [ ] Identifier le hotspot principal et mesurer avant/après.
- [ ] Réduire la complexité des modules les plus touchés.

### Mémoire chantier
- Control plane: `/Users/electron/.codex/memories/electron_rare_chantier`
- Repo card: `/Users/electron/.codex/memories/electron_rare_chantier/REPOS/linkyToVictron_oled.md`

<!-- CHANTIER:AUDIT END -->
