[🇬🇧 English](README.md) | [🇩🇪 Deutsch](README.de.md) | [🇳🇱 Nederlands](README.nl.md)

# Programmer-Decoder-POM
Software zur Konfiguration der DCC-Zubehördecoder mit RS-Bus-Rückmeldung, darunter die Weichen-, Relais-, Lift- und Servodecoder.

Mit diesem Programm können die [Weichendecoder](https://github.com/aikopras/OPENDECODER22) und verwandte Decoder, die auf der Bibliothek [AP_DCC_Decoder_Core](https://github.com/aikopras/AP_DCC_Decoder_Core) basieren, eingestellt werden, indem ihre CVs gelesen und geändert werden. Mit diesen Decodern werden Weichen, Relais und Servos gestellt. Jede Änderung der Stellung von Weichen und Servos wird über RS-Bus-Meldungen an die Zentrale zurückgemeldet. Da die Decoder den RS-Bus für die Rückmeldung verwenden, sind sie vor allem für Anlagen mit LENZ-Zentralen (wie der LZV 100) interessant, die über ein LAN/USB-Interface (wie das 23151) angeschlossen sind.

## CV-Werte lesen ##
Zum Einstellen und Ändern der CVs des Decoders sendet das Programm PoM-Befehle (Programming on the Main). Da die XpressNet-Spezifikation PoM-Befehle nur für Lokdecoder vorsieht (nicht für Zubehör- oder Rückmeldedecoder), hören die Decoder als „Trick“ auf die Lokadresse <I>Decoderadresse + 6999</I>. Die angefragten CV-Werte werden über den RS-Bus zurückgemeldet, mit der Adresse 128. Da alle Decoder diese Adresse für ihre PoM-Rückmeldungen teilen, müssen die Decoder einzeln eingestellt werden.

Das Programm kommuniziert mit der LENZ-Zentrale über das LAN/USB-Interface (23151). Werden für DCC-Befehle und RS-Bus-Rückmeldungen verschiedene Zentralen verwendet, können zwei LAN/USB-Interfaces benutzt werden.

## Startfenster ##
Das Programm startet mit einem Fenster, in dem die Weichenadresse eingegeben wird und einige Angaben zu diesem Decoder gezeigt werden. Die Reiter des Programms werden im [Handbuch](Manual/Manual.de.md) näher erklärt.
![Hauptfenster](Manual/Screenshots/Control.png)

## Download ##
| Version | Läuft auf | Programm und Installation | Quellcode und Übersetzen |
|---|---|---|---|
| macOS | macOS 12 oder neuer, Apple Silicon und Intel | [Compiled-Applications/macOS](Compiled-Applications/macOS/README.md) | [src/Xcode](src/Xcode/README.md) (Objective-C) |
| Windows | Windows 10 / 11 (64 Bit), Intel / AMD und ARM | [Compiled-Applications/Windows](Compiled-Applications/Windows/README.md) | [src/Qt](src/Qt/README.md) (C++ / Qt 6) |
| Linux | x86_64 | [Compiled-Applications/Linux](Compiled-Applications/Linux/README.md) | [src/Qt](src/Qt/README.md) (C++ / Qt 6) |

Die Änderungen pro Version stehen in der [Version history](Version_History.md) (Englisch).
