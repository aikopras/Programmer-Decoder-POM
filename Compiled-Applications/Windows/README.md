# Programmer-Decoder-POM for Windows (EN)
A description of the program itself is in the [main README](../../README.md). The source code and the build instructions are in [src/Qt](../../src/Qt/README.md).

## Requirements ##
- Windows 10 or 11, 64 bit.
- The program is built for x64 (Intel and AMD processors). It also runs on Windows 11 on ARM (for example in a virtual machine on an Apple silicon Mac), via the x64 emulation that is part of Windows.
- One or two Lenz LAN/USB interfaces (23151).

## Installation ##
1. Download [Programmer Decoder-POM.zip](Programmer%20Decoder-POM.zip) and unzip it to a location of your choice, for example `C:\Program Files` or your Documents folder. This gives the folder `Programmer Decoder-POM` with the program and the Qt files it needs; keep these files together.
2. Start `Programmer Decoder-POM.exe` in that folder. The program is not signed, so Windows SmartScreen may warn the first time. Click "More info" and then "Run anyway".
3. If Windows reports that `VCRUNTIME140.dll` or `MSVCP140.dll` is missing, install the Microsoft Visual C++ Redistributable (x64): [vc_redist.x64.exe](https://aka.ms/vs/17/release/vc_redist.x64.exe).
4. Enter the IP address(es) of the Lenz interface(s) via File → Preferences. They are stored in the registry (`HKEY_CURRENT_USER\Software\Programmer Decoder-POM\Settings`) and are used the next time the program connects.

### Shortcut on the desktop or in the Start menu ###
The folder contains many files; to start the program easily, make a shortcut to `Programmer Decoder-POM.exe`:
- Desktop: right-click `Programmer Decoder-POM.exe` and choose "Send to" → "Desktop (create shortcut)". On Windows 11 "Send to" is under "Show more options".
- Start menu: right-click `Programmer Decoder-POM.exe` and choose "Pin to Start".

The XpressNet library writes errors and warnings to a logfile in `%PUBLIC%\TMC-Logfiles`.

---

# Programmer-Decoder-POM für Windows (DE)
Eine Beschreibung des Programms selbst steht in der [Haupt-README](../../README.md). Quellcode und Anleitung zum Übersetzen stehen in [src/Qt](../../src/Qt/README.md).

## Voraussetzungen ##
- Windows 10 oder 11, 64 Bit.
- Das Programm ist für x64 (Intel- und AMD-Prozessoren) übersetzt. Es läuft auch unter Windows 11 auf ARM (zum Beispiel in einer virtuellen Maschine auf einem Mac mit Apple Silicon), über die x64-Emulation von Windows.
- Ein oder zwei Lenz LAN/USB-Interfaces (23151).

## Installation ##
1. [Programmer Decoder-POM.zip](Programmer%20Decoder-POM.zip) herunterladen und an einem beliebigen Ort entpacken, zum Beispiel in `C:\Program Files` oder im Ordner Dokumente. Dabei entsteht der Ordner `Programmer Decoder-POM` mit dem Programm und den benötigten Qt-Dateien; diese zusammen lassen.
2. `Programmer Decoder-POM.exe` in diesem Ordner starten. Das Programm ist nicht signiert; Windows SmartScreen warnt deshalb eventuell beim ersten Start. Auf „Weitere Informationen“ und dann auf „Trotzdem ausführen“ klicken.
3. Meldet Windows, dass `VCRUNTIME140.dll` oder `MSVCP140.dll` fehlt, das Microsoft Visual C++ Redistributable (x64) installieren: [vc_redist.x64.exe](https://aka.ms/vs/17/release/vc_redist.x64.exe).
4. Die IP-Adresse(n) der Lenz-Interfaces über File → Preferences eingeben. Sie werden in der Registry gespeichert (`HKEY_CURRENT_USER\Software\Programmer Decoder-POM\Settings`) und beim nächsten Verbindungsaufbau verwendet.

### Verknüpfung auf dem Desktop oder im Startmenü ###
Der Ordner enthält viele Dateien; um das Programm einfach zu starten, eine Verknüpfung zu `Programmer Decoder-POM.exe` anlegen:
- Desktop: mit der rechten Maustaste auf `Programmer Decoder-POM.exe` klicken und „Senden an“ → „Desktop (Verknüpfung erstellen)“ wählen. Unter Windows 11 steht „Senden an“ unter „Weitere Optionen anzeigen“.
- Startmenü: mit der rechten Maustaste auf `Programmer Decoder-POM.exe` klicken und „An Start anheften“ wählen.

Die XpressNet-Bibliothek schreibt Fehler und Warnungen in eine Logdatei in `%PUBLIC%\TMC-Logfiles`.

---

# Programmer-Decoder-POM voor Windows (NL)
Een beschrijving van het programma zelf staat in de [hoofd-README](../../README.md). De broncode en de bouwinstructies staan in [src/Qt](../../src/Qt/README.md).

## Vereisten ##
- Windows 10 of 11, 64 bit.
- Het programma is gebouwd voor x64 (Intel- en AMD-processoren). Het draait ook op Windows 11 op ARM (bijvoorbeeld in een virtuele machine op een Mac met Apple silicon), via de x64-emulatie die in Windows zit.
- Eén of twee Lenz LAN/USB-interfaces (23151).

## Installatie ##
1. Download [Programmer Decoder-POM.zip](Programmer%20Decoder-POM.zip) en pak het uit op een plek naar keuze, bijvoorbeeld in `C:\Program Files` of in de map Documenten. Daarbij ontstaat de map `Programmer Decoder-POM` met het programma en de Qt-bestanden die het nodig heeft; houd die bij elkaar.
2. Start `Programmer Decoder-POM.exe` in die map. Het programma is niet gesigneerd; Windows SmartScreen kan daarom de eerste keer waarschuwen. Klik op "Meer informatie" en daarna op "Toch uitvoeren".
3. Meldt Windows dat `VCRUNTIME140.dll` of `MSVCP140.dll` ontbreekt, installeer dan de Microsoft Visual C++ Redistributable (x64): [vc_redist.x64.exe](https://aka.ms/vs/17/release/vc_redist.x64.exe).
4. Voer het IP-adres (of de IP-adressen) van de Lenz-interface(s) in via File → Preferences. Ze worden bewaard in het register (`HKEY_CURRENT_USER\Software\Programmer Decoder-POM\Settings`) en gebruikt bij de volgende verbinding.

### Snelkoppeling op het bureaublad of in het Startmenu ###
De map bevat veel bestanden; maak een snelkoppeling naar `Programmer Decoder-POM.exe` om het programma makkelijk te starten:
- Bureaublad: klik met de rechtermuisknop op `Programmer Decoder-POM.exe` en kies "Kopiëren naar" → "Bureaublad (snelkoppeling maken)". Op Windows 11 staat "Kopiëren naar" onder "Meer opties weergeven".
- Startmenu: klik met de rechtermuisknop op `Programmer Decoder-POM.exe` en kies "Aan Start vastmaken".

De XpressNet-library schrijft fouten en waarschuwingen naar een logfile in `%PUBLIC%\TMC-Logfiles`.
