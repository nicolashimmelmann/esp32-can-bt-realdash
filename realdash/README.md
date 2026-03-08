# RealDash files

## Dateien

- `custom_vehicle.xml` - erste Kanalbeschreibung für virtuelle ESP32-Frames

## Konzept

Die XML geht von **virtuellen CAN-Frames** aus, die der ESP32 selbst für RealDash erzeugt.

Aktuelle Frame-Belegung:

- `0x500`
  - Engine RPM
  - Vehicle Speed
  - Coolant Temp
  - Battery Voltage

- `0x501`
  - Throttle Position
  - Intake Air Temp
  - Manifold Pressure
  - Fuel Level

- `0x502`
  - Oil Temp
  - Oil Pressure
  - Boost Pressure
  - CAN Status

- `0x503`
  - Statusbits für Warnlampen und Schalter

## Import in RealDash

In RealDash:

- `Garage -> Connections`
- neue Verbindung hinzufügen
- Typ `RealDash CAN`
- Transport `Bluetooth`
- ESP32 auswählen
- bei `Custom Channel Description File` die Datei `custom_vehicle.xml` wählen

## Wichtig

Diese XML ist ein **Start-Template**.

Die Firmware muss dazu passende virtuelle Frames mit genau dieser Byte-Belegung senden. Alternativ wird die XML später an die echte Firmware-Belegung angepasst.
