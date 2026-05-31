# Solarschirm Steuerung

Programmiert für und mit ESP-IDF 5.5.1

## Voraussetzungen

- [ESP-IDF 5.5.1](https://docs.espressif.com/projects/esp-idf/en/v5.5.1/esp32/get-started/index.html) installiert und konfiguriert
- Unterstützte Targets: ESP32 / ESP32-C3 / ESP32-S3

## Einstellungen

### Bluetooth – NimBLE aktivieren

Vor dem ersten Build muss ggf. NimBLE als Bluetooth-Stack eingestellt werden:

```bash
idf.py menuconfig
```

Dann navigieren zu:

```
Component config → Bluetooth → Bluetooth Host → NimBLE
```

## Build & Flash


### Port konfigurieren
```bash
# Verfügbare Ports anzeigen
# Linux/macOS:
ls /dev/tty*
# Windows: Geräte-Manager → Anschlüsse (COM & LPT)

Den Port `/dev/ttyUSB0` entsprechend anpassen (Windows: `COM3`, macOS: `/dev/cu.usbserial-...`)

# Projekt konfigurieren (Target setzen, einmalig)
idf.py set-target esp32
```
### Das Projekt kann entweder über den Flammen-Button (unten Mitte) oder über die Console gebaut werden.

```bash
idf.py build

# Bauen
# Flashen
idf.py -p /dev/ttyUSB0 flash

# Monitor (optional)
idf.py -p /dev/ttyUSB0 monitor

# Flash + Monitor kombiniert
idf.py -p /dev/ttyUSB0 flash monitor

```
