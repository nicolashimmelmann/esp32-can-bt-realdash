# ESP32 + MCP2515 + Bluetooth + RealDash Plan

## Ziel

Ein ESP32 liest definierte Werte vom Fahrzeug-CAN-Bus über ein MCP2515-Modul, verpackt diese in das von RealDash erwartete `RealDash CAN`-Format und sendet sie per Bluetooth an die RealDash-App. Parallel wird eine RealDash-Konfiguration erstellt, damit die Werte im Dashboard korrekt angezeigt werden.

## Architekturentscheidung

- **Mikrocontroller:** ESP32
- **CAN-Controller:** MCP2515 per SPI
- **Bluetooth-Link zu RealDash:** ESP32 **Bluetooth Classic SPP**
- **RealDash-Verbindungstyp:** `RealDash CAN`
- **Datenmodell zwischen ESP32 und RealDash:** Keine rohen Bluetooth-Strings, sondern RealDash-CAN-Frames
- **Mapping der Werte in RealDash:** XML-Datei mit Frame-IDs, Offsets, Bitfeldern und Conversion-Formeln

## Wichtige Recherche-Ergebnisse

### 1. RealDash erwartet eigenes CAN-Transportformat

Für die Verbindung `RealDash CAN` erwartet RealDash keinen nackten MCP2515-Dump, sondern eigene Frames.

Das wichtigste Basisformat ist:

- 4 Byte Header: `0x44 0x33 0x22 0x11`
- 4 Byte CAN-ID: **32-bit little endian**
- 8 Byte Payload

Das bedeutet:

- Der ESP32 muss aus empfangenen CAN-Nachrichten ein RealDash-kompatibles Paket bauen.
- Auch bei kürzeren CAN-Nutzdaten muss auf 8 Byte gepolstert werden.

### 2. RealDash-Mapping läuft über XML

Die Anzeige in RealDash entsteht nicht automatisch aus der CAN-ID. Stattdessen beschreibt eine XML-Datei:

- welche CAN-ID relevant ist
- an welchem Byte/Bit ein Wert liegt
- ob signed/unsigned
- welche Endianness gilt
- welche Umrechnung nötig ist
- ob auf eingebaute RealDash-`targetId`s oder benannte Custom-Werte gemappt wird

### 3. Bluetooth: Classic SPP bevorzugen

Für diesen Use-Case ist **Bluetooth Classic SPP** der beste Pfad.

Gründe:

- ESP32 unterstützt das in Arduino direkt über `BluetoothSerial`
- RealDash-Beispiele nennen Bluetooth/Serial/WiFi als typische Transportwege
- BLE ist laut RealDash-Forum deutlich fehleranfälliger bzw. unklarer als serieller Transport

### 4. MCP2515-Risiken vor eigentlicher Firmware klären

Vor der Implementierung müssen diese Hardwareparameter sicher bekannt sein:

- **CAN-Bitrate** des Fahrzeugs, z. B. `500 kbps`, `250 kbps`, `125 kbps`
- **Quarzfrequenz** des MCP2515-Moduls, meist `8 MHz` oder `16 MHz`
- **Spannungspegel / Board-Variante** des MCP2515-Moduls
- ob der Ziel-CAN-Bus wirklich Standard-CAN mit 11-bit IDs nutzt oder Extended IDs vorkommen

Wenn Bitrate oder Quarz falsch gesetzt sind, wirkt der Bus „tot“.

## Implementierungsplan

## Phase 1 - Hardware- und Bus-Basis festlegen

### Schritt 1.1 - Zielbus definieren

Vor jeder Firmwarearbeit festlegen:

- an welchem CAN-Bus gelesen wird
- welche Werte gebraucht werden
- ob diese Werte schon bekannt sind oder erst gesnifft werden müssen

Ergebnis dieser Phase:

- Liste gewünschter Werte, z. B. RPM, Geschwindigkeit, Kühlmitteltemp, Drosselklappe, Batteriespannung
- bekannte oder vermutete CAN-IDs
- bekannte oder vermutete Skalierung/Offset

### Schritt 1.2 - Verdrahtung festlegen

Empfohlene Standardverdrahtung ESP32 ↔ MCP2515:

- `GPIO23` -> `MOSI`
- `GPIO19` -> `MISO`
- `GPIO18` -> `SCK`
- `GPIO5` -> `CS`
- `GPIO4` -> `INT`
- `3.3V` -> Modul-Versorgung, **nur wenn das konkrete Modul 3.3V-tauglich ist**
- `GND` -> `GND`

CAN-Seite:

- `CANH` -> Fahrzeug `CANH`
- `CANL` -> Fahrzeug `CANL`
- gemeinsame Masse sauber prüfen

### Schritt 1.3 - MCP2515-Modul identifizieren

Dokumentieren:

- Board-Typ
- Quarz `8 MHz` oder `16 MHz`
- Transceiver-Typ, z. B. `TJA1050`
- Versorgungsspannung des Boards
- ob Pegelwandler auf dem Modul vorhanden sind

Ohne diese Daten keine finale CAN-Initialisierung fest verdrahten.

## Phase 2 - Firmware-Grundgerüst aufbauen

### Schritt 2.1 - Arduino-Projektstruktur definieren

Geplante Dateien:

- `src/main.ino` oder `esp32_can_realdash.ino`
- `src/config.h`
- `src/can_reader.*`
- `src/realdash_protocol.*`
- `src/bluetooth_link.*`
- `src/signal_decoder.*`

Ziel:

- CAN lesen getrennt von RealDash-Transport
- Signaldecoding getrennt vom Transport
- Konfigurationen zentral pflegen

### Schritt 2.2 - Bibliotheken festlegen

Voraussichtlich nötig:

- ESP32 Arduino Core
- `BluetoothSerial.h`
- eine ESP32-kompatible MCP2515-Library

Vor Implementierung prüfen, welche konkrete Library stabil mit ESP32 + MCP2515 + gewünschter Clock arbeitet.

### Schritt 2.3 - Minimaltest: Bluetooth SPP

Erstes Firmwareziel:

- ESP32 startet als Bluetooth-Gerät mit festem Namen, z. B. `ESP32-RealDash`
- Android/Tablet kann pairen
- serielle Testdaten werden gesendet

Abnahmekriterium:

- RealDash bzw. Test-App kann den SPP-Stream stabil öffnen

## Phase 3 - CAN-Kommunikation stabil machen

### Schritt 3.1 - Minimaltest: CAN-Rohframes lesen

Firmware zunächst nur auf MCP2515-Initialisierung und CAN-Empfang fokussieren.

Ziele:

- CAN-Bus erfolgreich starten
- empfangene IDs und Daten loggen
- Fehlerzähler / Empfangsstatus sichtbar machen

Abnahmekriterium:

- auf dem seriellen Monitor erscheinen plausible CAN-Frames mit stabiler Frequenz

### Schritt 3.2 - Filterstrategie definieren

Erst breit lauschen, dann reduzieren:

- Start ohne enge Filter
- relevante IDs identifizieren
- später MCP2515-Filter/Masks setzen, um CPU-Last zu senken

### Schritt 3.3 - Zielsignale decodieren

Für jeden gewünschten Wert dokumentieren:

- CAN-ID
- DLC / Payload-Länge
- Byte-Offset oder Bitposition
- Endianness
- signed/unsigned
- Umrechnung

Empfohlene Artefakte:

- `signals.md` oder Tabelle im README
- Decoder-Funktionen pro Signalgruppe

## Phase 4 - RealDash-Transport implementieren

### Schritt 4.1 - RealDash-44-Frame Builder schreiben

Eine Funktion implementieren, die aus einer CAN-Nachricht erzeugt:

- Header `44 33 22 11`
- CAN-ID little-endian
- 8 Byte Payload

Regeln:

- Standard-CAN-Nachrichten direkt verpacken
- bei Payload < 8 Byte mit `0x00` auffüllen
- optional Extended-ID-Unterstützung bewusst testen, falls Fahrzeug das nutzt

### Schritt 4.2 - Sende-Strategie festlegen

Nicht blind jeden Busframe weiterreichen, sondern bewusst planen:

Option A:

- nur relevante Original-CAN-Frames an RealDash weitergeben

Option B:

- Werte intern decodieren und in eigene virtuelle CAN-Frames mit eigenen IDs schreiben

Empfehlung:

- **Für den Start Option B**, weil damit das RealDash-XML einfacher und stabiler wird
- z. B. eigene IDs wie `0x500`, `0x501`, `0x502` nur für Dashboard-Werte definieren

Vorteile:

- unabhängig von OEM-Frame-Komplexität
- einfache Offsets
- weniger XML-Komplexität
- spätere Erweiterungen leichter

### Schritt 4.3 - Update-Intervalle definieren

Für jede Wertgruppe feste Sendefrequenzen definieren:

- schnelle Werte wie RPM / Geschwindigkeit: `20-50 ms`
- mittlere Werte wie Temperaturen: `100-500 ms`
- langsame Zustände / Flags: `100-250 ms`

Ziel:

- Dashboard flüssig
- Bluetooth nicht unnötig überlastet

## Phase 5 - RealDash-XML erstellen

### Schritt 5.1 - XML-Grunddatei anlegen

Datei z. B.:

- `realdash/custom_vehicle.xml`

Struktur:

- `<RealDashCAN>`
- `<frames>`
- pro Frame eine `<frame id="...">`
- pro Wert ein `<value ... />`

### Schritt 5.2 - Eingebaute `targetId`s vs. Custom-Namen entscheiden

Zwei Wege:

- **eingebaute RealDash targetIds** für Standardwerte wie RPM, Speed, Coolant
- **Custom names** für fahrzeugspezifische Daten

Empfehlung:

- Standardanzeigen auf bekannte `targetId`s mappen
- nur Spezialwerte als Custom Inputs anlegen

### Schritt 5.3 - Conversion in XML oder Firmware entscheiden

Empfehlung:

- wenn möglich **Umrechnung schon in der Firmware** erledigen und einfache Integer-Werte senden
- XML nur für leichte Korrekturen nutzen

Grund:

- Debugging einfacher
- Dashboard bleibt verständlicher
- gleiche Werte können leichter in anderen Clients genutzt werden

## Phase 6 - RealDash-App einrichten

### Schritt 6.1 - Verbindung aufbauen

In RealDash:

- `Garage -> Connections`
- neue Verbindung hinzufügen
- Typ `RealDash CAN`
- Transport `Bluetooth`
- ESP32 auswählen
- Custom Channel Description File = eigene XML auswählen

### Schritt 6.2 - Werte prüfen

Erst mit wenigen sicheren Werten testen:

- RPM
- Geschwindigkeit
- Kühlmitteltemperatur
- Batteriespannung

Abnahmekriterium:

- Werte erscheinen plausibel
- keine extremen Sprünge
- keine vertauschten Byte-Reihenfolgen

### Schritt 6.3 - Dashboard aufbauen

Danach:

- bestehendes Dashboard anpassen oder neues Dashboard anlegen
- Gauges mit gemappten Inputs verbinden
- Warnungen/Trigger für Grenzwerte definieren

## Phase 7 - Debugging- und Teststrategie

### Schritt 7.1 - Stufenweise testen

Testreihenfolge strikt einhalten:

1. ESP32 startet
2. Bluetooth SPP funktioniert
3. MCP2515 initialisiert korrekt
4. CAN-Rohframes kommen rein
5. relevante Signale sind identifiziert
6. RealDash-Frames werden erzeugt
7. RealDash zeigt Werte an

### Schritt 7.2 - Logging einbauen

Firmware sollte in Debug-Builds loggen:

- Bluetooth verbunden/getrennt
- CAN init ok/fehlerhaft
- empfangene relevante CAN-IDs
- gesendete RealDash-Frames pro Sekunde
- Buffer-/Overflow-Situationen

### Schritt 7.3 - Häufige Fehler explizit prüfen

Checkliste:

- falsche MCP2515-Clock `8 MHz` vs `16 MHz`
- falsche CAN-Bitrate
- falsche SPI-Pins
- Modul nicht 3.3V-kompatibel
- Fahrzeug nutzt andere CAN-ID als erwartet
- falsche Endianness
- Wert ist signed statt unsigned
- RealDash-XML referenziert falsche Offsets/Bits
- Tablet verbindet per BLE statt Bluetooth Classic

## Phase 8 - Konkrete Implementierungsreihenfolge

### Milestone A - Kommunikationsbasis

Implementieren:

- ESP32-Projektgerüst
- Bluetooth Classic SPP
- MCP2515-Init
- Roh-CAN-Empfang

Fertig wenn:

- Busdaten sichtbar
- Bluetooth-Verbindung stabil

### Milestone B - Datenpfad zu RealDash

Implementieren:

- RealDash-Frame-Builder
- Auswahl relevanter CAN-Frames
- periodisches Senden per Bluetooth

Fertig wenn:

- RealDash verbindet
- mindestens ein Testwert erscheint

### Milestone C - Signalmodell

Implementieren:

- decodierte Zielwerte
- eigene virtuelle RealDash-CAN-Frames
- strukturierte XML-Datei

Fertig wenn:

- mehrere Werte korrekt im Dashboard stehen

### Milestone D - Produktiv machen

Implementieren:

- Filter
- Reconnect-Handling
- Timeouts / Offline-Werte
- robustes Logging
- finale Dashboard-Seite

Fertig wenn:

- System im Fahrzeug reproduzierbar läuft

## Empfohlene erste Umsetzung

Als erstes nicht direkt alle Fahrzeugdaten anbinden, sondern diesen Minimalpfad umsetzen:

1. ESP32 + Bluetooth Classic SPP starten
2. MCP2515 lesen
3. einen bekannten CAN-Wert finden oder testweise einen festen virtuellen Wert erzeugen
4. diesen in **eine eigene virtuelle CAN-ID** für RealDash verpacken
5. XML mit genau einem Wert erstellen
6. Anzeige in RealDash verifizieren
7. danach weitere Werte ergänzen

Das minimiert Debug-Aufwand massiv.

## Offene Punkte, die vor dem Coding beantwortet werden sollten

- Welche **genauen Werte** sollen zuerst angezeigt werden?
- Von welchem **Fahrzeug / Steuergerät / CAN-Bus** kommen sie?
- Ist die **CAN-Bitrate** bekannt?
- Sind die **Signal-IDs und Skalierungen** schon bekannt oder müssen sie erst ermittelt werden?
- Welches **MCP2515-Board** ist konkret vorhanden (`8 MHz` oder `16 MHz`)?
- Soll RealDash auf **Android** oder **iOS** laufen? Bluetooth Classic ist vor allem für Android der naheliegende Pfad.

## Nächster sinnvoller Schritt

Nach diesem Plan als nächstes implementieren:

- ein minimales Arduino-Projekt für ESP32 + MCP2515 + Bluetooth Classic
- eine erste Test-XML für 1-2 Werte
- optional eine CAN-Sniffer-Hilfe zum Identifizieren der relevanten IDs
