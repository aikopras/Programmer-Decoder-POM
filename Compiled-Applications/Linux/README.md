# Programmer-Decoder-POM for Linux (EN)
A description of the program itself is in the [main README](../../README.md). The source code and the build instructions are in [src/Qt](../../src/Qt/README.md).

## Installation ##
[Programmer Decoder-POM.zip](Programmer%20Decoder-POM.zip) contains the program, built on Ubuntu 24.04 (x86_64) with Qt 6.4.2. It uses the Qt 6 libraries of the system. On Ubuntu / Debian these are installed with:

    sudo apt install libqt6widgets6 libqt6network6 libqt6serialport6

Unzip it (`unzip "Programmer Decoder-POM.zip"`) and start the program with `./"Programmer Decoder-POM"`. If the program is not executable, use `chmod +x "Programmer Decoder-POM"` first. On other distributions, or if the program does not start, build it from source (see [src/Qt](../../src/Qt/README.md)).

Enter the IP address(es) of the Lenz interface(s) via File → Preferences. They are stored in `~/.config/Programmer Decoder-POM/Settings.conf` and are used the next time the program connects. The XpressNet library writes errors and warnings to a logfile in `~/TMC-Logfiles`.

---

# Programmer-Decoder-POM für Linux (DE)
Eine Beschreibung des Programms selbst steht in der [Haupt-README](../../README.md). Quellcode und Anleitung zum Übersetzen stehen in [src/Qt](../../src/Qt/README.md).

## Installation ##
[Programmer Decoder-POM.zip](Programmer%20Decoder-POM.zip) enthält das Programm, übersetzt unter Ubuntu 24.04 (x86_64) mit Qt 6.4.2. Es verwendet die Qt-6-Bibliotheken des Systems. Unter Ubuntu / Debian werden diese installiert mit:

    sudo apt install libqt6widgets6 libqt6network6 libqt6serialport6

Entpacken (`unzip "Programmer Decoder-POM.zip"`) und das Programm mit `./"Programmer Decoder-POM"` starten. Ist das Programm nicht ausführbar, zuerst `chmod +x "Programmer Decoder-POM"` eingeben. Bei anderen Distributionen, oder wenn das Programm nicht startet, selbst übersetzen (siehe [src/Qt](../../src/Qt/README.md)).

Die IP-Adresse(n) der Lenz-Interfaces über File → Preferences eingeben. Sie werden in `~/.config/Programmer Decoder-POM/Settings.conf` gespeichert und beim nächsten Verbindungsaufbau verwendet. Die XpressNet-Bibliothek schreibt Fehler und Warnungen in eine Logdatei in `~/TMC-Logfiles`.

---

# Programmer-Decoder-POM voor Linux (NL)
Een beschrijving van het programma zelf staat in de [hoofd-README](../../README.md). De broncode en de bouwinstructies staan in [src/Qt](../../src/Qt/README.md).

## Installatie ##
[Programmer Decoder-POM.zip](Programmer%20Decoder-POM.zip) bevat het programma, gebouwd op Ubuntu 24.04 (x86_64) met Qt 6.4.2. Het gebruikt de Qt-6-libraries van het systeem. Op Ubuntu / Debian worden die geïnstalleerd met:

    sudo apt install libqt6widgets6 libqt6network6 libqt6serialport6

Pak het uit (`unzip "Programmer Decoder-POM.zip"`) en start het programma met `./"Programmer Decoder-POM"`. Is het programma niet uitvoerbaar, geef dan eerst `chmod +x "Programmer Decoder-POM"`. Bouw het zelf (zie [src/Qt](../../src/Qt/README.md)) bij andere distributies, of als het programma niet start.

Voer het IP-adres (of de IP-adressen) van de Lenz-interface(s) in via File → Preferences. Ze worden bewaard in `~/.config/Programmer Decoder-POM/Settings.conf` en gebruikt bij de volgende verbinding. De XpressNet-library schrijft fouten en waarschuwingen naar een logfile in `~/TMC-Logfiles`.
