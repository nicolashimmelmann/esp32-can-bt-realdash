# ESP32 CAN-BT Dashboard

ESP32-based CAN bus to Bluetooth bridge for RealDash compatibility.

## Features

- CAN bus communication via MCP2515
- Bluetooth connectivity for mobile dashboard apps
- RealDash protocol support
- Simple frame utilities

## Setup

1. Install PlatformIO
2. Connect hardware according to pin configuration
3. Upload firmware

Der ESP32 liest CAN-Frames über MCP2515, verpackt sie in das `RealDash CAN`-Format und sendet sie per Bluetooth an RealDash.

## Projektstruktur

- `platformio.ini` - PlatformIO-Konfiguration
- `include/config.h` - zentrale Pins und CAN-Konfiguration
- `include/can_bus.h` - CAN-Abstraktion
- `include/bluetooth_link.h` - Bluetooth-SPP-Abstraktion
- `include/realdash_protocol.h` - RealDash-Transportformat
- `src/main.cpp` - Hauptloop
- `src/can_bus.cpp` - MCP2515-Initialisierung und Lesen
- `src/bluetooth_link.cpp` - Bluetooth-Transport
- `src/realdash_protocol.cpp` - `44`-Frame-Encoding für RealDash
- `realdash/custom_vehicle.xml` - erste RealDash-Kanalbeschreibung
- `realdash/README.md` - Hinweise zu Frame-Belegung und Import
- `plan.md` - Umsetzungsplan

## Aktueller Stand

Das Projekt enthält ein minimales Grundgerüst:

- startet Bluetooth Classic mit Geräte-Namen aus `config.h`
- initialisiert MCP2515
- enthält einen Normalbetrieb für echte CAN-Frames via MCP2515
- erzeugt aktuell virtuelle RealDash-Testframes `0x500` bis `0x503`
- sendet diese bei aktiver Bluetooth-Verbindung passend zur XML weiter
- enthält eine erste RealDash-XML für virtuelle ESP32-Frames

## Wichtige Konfiguration

In `include/config.h` prüfen/anpassen:

- `kBluetoothDeviceName`
- `kRunMode`
- SPI-Pins
- `kCanChipSelectPin`
- `kCanInterruptPin`
- `kCanSpeed`
- `kCanClockMHz`
- `kForwardedFrameIds`

## Betriebsmodi

In `include/config.h` umschaltbar:

- `config::RunMode::kRealDashTestFrames`
  - sendet virtuelle Testframes `0x500` bis `0x503`
  - passt direkt zur `realdash/custom_vehicle.xml`

- `config::RunMode::kNormalOperation`
  - liest echte CAN-Frames vom MCP2515
  - filtert auf Standard-Frames in `kForwardedFrameIds`
  - leitet passende Frames als RealDash-`44`-Frames per Bluetooth weiter

- `config::RunMode::kCanSniffer`
  - liest echte CAN-Frames vom MCP2515
  - gibt sie auf dem seriellen Monitor aus
  - leitet Standard-Frames optional weiterhin als RealDash-`44`-Frames per Bluetooth weiter

## RealDash XML

Die erste XML liegt unter:

- `realdash/custom_vehicle.xml`

Sie beschreibt aktuell **virtuelle CAN-Frames** mit den IDs:

- `0x500`
- `0x501`
- `0x502`
- `0x503`

Wichtig:

- Die aktuelle Firmware sendet bereits passende virtuelle Testframes.
- Als nächster Schritt ersetzen wir diese Testwerte durch echte, aus CAN decodierte Werte.
- Die XML kann vorerst unverändert bleiben, solange die Byte-Belegung gleich bleibt.

**Hinweis:** Target IDs für RealDash können unter https://realdash.net/manuals/targetid.php gefunden werden.

## Standard-Pinout

Aktuell hinterlegt:

- `SCK = GPIO18`
- `MISO = GPIO19`
- `MOSI = GPIO23`
- `CS = GPIO5`
- `INT = GPIO4`

## Wichtige Hinweise

- `kCanClockMHz` muss zum MCP2515-Quarz passen, meist `8` oder `16`.
- `kCanSpeed` muss zur Fahrzeug-Bitrate passen.
- Im Normalbetrieb leitet die Firmware gefilterte Standard-CAN-Frames direkt an RealDash weiter.
- Im Sniffer-Modus kannst du echte Fahrzeug-Frames auf dem seriellen Monitor sehen.
- Im Testframe-Modus sendet die Firmware weiter **virtuelle Testframes**.
- Das ist weiter ein Startgerüst, noch keine finale Signalzuordnung für ein konkretes Fahrzeug.

## PlatformIO

Beispiel:

```bash
pio run
pio run -t upload
pio device monitor
```

## Nächster Schritt

Als Nächstes sinnvoll:

- konkretes Board prüfen
- CAN-Bitrate und MCP2515-Quarz bestätigen
- im Sniffer-Modus relevante CAN-IDs finden
- danach echte CAN-Signale dekodieren und die Testwerte ersetzen
