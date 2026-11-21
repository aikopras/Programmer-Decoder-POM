# Source code of the macOS version (EN)
The compiled program and the installation instructions are in [Compiled-Applications/macOS](../../Compiled-Applications/macOS/README.md). A description of the program itself is in the [main README](../../README.md).

## Building from source ##
Open `Programmer Decoder-POM.xcodeproj` (in this directory) in Xcode 26 or later. The Release configuration (for example via Product → Build For → Profiling, or Product → Archive) produces a universal binary for Apple silicon and Intel. The help files are described in [Helpfiles/readme.txt](Helpfiles/readme.txt); the Windows and Linux versions ([src/Qt](../Qt/README.md)) use the same help files.

The TCP connections are handled by `TCPSocketClass` (`TCPSocket.h` / `TCPSocket.m`), which is based on Apple's Network framework. This class contains no DCC, RS-Bus or Lenz specific code; the same class is used in [OSX-DCC-Monitor](https://github.com/aikopras/OSX-DCC-Monitor).

---

# Quellcode der macOS-Version (DE)
Das übersetzte Programm und die Installationsanleitung stehen in [Compiled-Applications/macOS](../../Compiled-Applications/macOS/README.md). Eine Beschreibung des Programms selbst steht in der [Haupt-README](../../README.md).

## Selbst übersetzen ##
`Programmer Decoder-POM.xcodeproj` (in diesem Verzeichnis) in Xcode 26 oder neuer öffnen. Die Release-Konfiguration (zum Beispiel über Product → Build For → Profiling oder Product → Archive) erzeugt ein Universal Binary für Apple Silicon und Intel. Die Hilfedateien sind in [Helpfiles/readme.txt](Helpfiles/readme.txt) beschrieben (Englisch); die Windows- und Linux-Versionen ([src/Qt](../Qt/README.md)) verwenden dieselben Hilfedateien.

Die TCP-Verbindungen werden von `TCPSocketClass` (`TCPSocket.h` / `TCPSocket.m`) abgewickelt, basierend auf dem Network-Framework von Apple. Diese Klasse enthält keinen DCC-, RS-Bus- oder Lenz-spezifischen Code; dieselbe Klasse wird in [OSX-DCC-Monitor](https://github.com/aikopras/OSX-DCC-Monitor) verwendet.

---

# Broncode van de macOS-versie (NL)
Het gecompileerde programma en de installatie-instructies staan in [Compiled-Applications/macOS](../../Compiled-Applications/macOS/README.md). Een beschrijving van het programma zelf staat in de [hoofd-README](../../README.md).

## Zelf bouwen ##
Open `Programmer Decoder-POM.xcodeproj` (in deze directory) in Xcode 26 of nieuwer. De Release-configuratie (bijvoorbeeld via Product → Build For → Profiling, of Product → Archive) maakt een universal binary voor Apple silicon en Intel. De helpbestanden zijn beschreven in [Helpfiles/readme.txt](Helpfiles/readme.txt) (Engels); de Windows- en Linux-versies ([src/Qt](../Qt/README.md)) gebruiken dezelfde helpbestanden.

De TCP-verbindingen worden afgehandeld door `TCPSocketClass` (`TCPSocket.h` / `TCPSocket.m`), gebaseerd op het Network framework van Apple. Deze class bevat geen DCC-, RS-Bus- of Lenz-specifieke code; dezelfde class wordt gebruikt in [OSX-DCC-Monitor](https://github.com/aikopras/OSX-DCC-Monitor).
