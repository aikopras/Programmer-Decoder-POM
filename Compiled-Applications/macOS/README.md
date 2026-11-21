# Programmer-Decoder-POM for macOS (EN)
A description of the program itself is in the [main README](../../README.md). The source code and the build instructions are in [src/Xcode](../../src/Xcode/README.md).

## Requirements ##
- macOS 12 (Monterey) or later.
- Universal binary: runs natively on Apple silicon Macs and on Intel Macs.
- One or two Lenz LAN/USB interfaces (23151).

## Installation ##
1. Download [Programmer Decoder-POM.zip](Programmer%20Decoder-POM.zip), unzip it and move the application to the Applications folder.
2. The application is not notarized by Apple, so macOS blocks it the first time it is opened. To allow it, open System Settings → Privacy & Security (on macOS 12: System Preferences → Security & Privacy → General) and click "Open Anyway".
3. On recent macOS versions the system may ask whether the application may access devices on the local network. Allow this; otherwise the TCP connections to the Lenz interface(s) fail.

## Connections to the Lenz interface(s) ##
When the program starts, it connects to the Lenz interface(s). If an interface does not respond within 3 seconds (for example because it is switched off), the program stops trying and shows this in the status line. The connection is made again as soon as a decoder address is entered (followed by Return) or SELECT is pressed; if the connection already exists, nothing changes.

As described in the Lenz specification of the LAN/USB Interface 23151 (section 2.1), the interface stops sending broadcasts (including the RS-Bus feedback messages) to a LAN device it has not heard from for some time. Therefore the program sends the XpressNet "Interface Status" command every 10 seconds, but only when no other command is pending. If an interface does not answer within 5 seconds, the connection is closed and this is shown in the status line.

---

# Programmer-Decoder-POM für macOS (DE)
Eine Beschreibung des Programms selbst steht in der [Haupt-README](../../README.md). Quellcode und Anleitung zum Übersetzen stehen in [src/Xcode](../../src/Xcode/README.md).

## Voraussetzungen ##
- macOS 12 (Monterey) oder neuer.
- Universal Binary: läuft nativ auf Macs mit Apple Silicon und auf Intel-Macs.
- Ein oder zwei Lenz LAN/USB-Interfaces (23151).

## Installation ##
1. [Programmer Decoder-POM.zip](Programmer%20Decoder-POM.zip) herunterladen, entpacken und das Programm in den Ordner Programme verschieben.
2. Das Programm ist nicht von Apple notarisiert; macOS blockiert es deshalb beim ersten Öffnen. Zum Zulassen in Systemeinstellungen → Datenschutz & Sicherheit (macOS 12: Systemeinstellungen → Sicherheit → Allgemein) auf „Dennoch öffnen“ klicken.
3. Neuere macOS-Versionen fragen eventuell, ob das Programm auf Geräte im lokalen Netzwerk zugreifen darf. Dies erlauben, sonst scheitern die TCP-Verbindungen zu den Lenz-Interfaces.

## Verbindungen zu den Lenz-Interfaces ##
Beim Start verbindet sich das Programm mit den Lenz-Interfaces. Antwortet ein Interface nicht innerhalb von 3 Sekunden (zum Beispiel weil es ausgeschaltet ist), gibt das Programm auf und zeigt dies in der Statuszeile. Die Verbindung wird erneut aufgebaut, sobald eine Decoderadresse eingegeben (mit Return) oder SELECT gedrückt wird; besteht die Verbindung bereits, ändert sich nichts.

Wie in der Lenz-Spezifikation des LAN/USB-Interface 23151 (Abschnitt 2.1) beschrieben, sendet das Interface keine Broadcasts (also auch keine RS-Bus-Rückmeldungen) mehr an ein LAN-Gerät, von dem es eine Zeit lang nichts gehört hat. Darum sendet das Programm alle 10 Sekunden den XpressNet-Befehl „Interface Status“, aber nur, wenn kein anderer Befehl aussteht. Antwortet ein Interface nicht innerhalb von 5 Sekunden, wird die Verbindung geschlossen und dies in der Statuszeile angezeigt.

---

# Programmer-Decoder-POM voor macOS (NL)
Een beschrijving van het programma zelf staat in de [hoofd-README](../../README.md). De broncode en de bouwinstructies staan in [src/Xcode](../../src/Xcode/README.md).

## Vereisten ##
- macOS 12 (Monterey) of nieuwer.
- Universal binary: draait native op Macs met Apple silicon en op Intel-Macs.
- Eén of twee Lenz LAN/USB-interfaces (23151).

## Installatie ##
1. Download [Programmer Decoder-POM.zip](Programmer%20Decoder-POM.zip), pak het uit en zet het programma in de map Apps (Programma's).
2. Het programma is niet door Apple genotariseerd; macOS blokkeert het daarom de eerste keer dat het geopend wordt. Om het toe te staan: open Systeeminstellingen → Privacy en beveiliging (op macOS 12: Systeemvoorkeuren → Beveiliging en privacy → Algemeen) en klik op "Toch openen".
3. Op recente macOS-versies kan het systeem vragen of het programma apparaten in het lokale netwerk mag benaderen. Sta dit toe, anders mislukken de TCP-verbindingen met de Lenz-interface(s).

## Verbindingen met de Lenz-interface(s) ##
Bij het starten maakt het programma verbinding met de Lenz-interface(s). Reageert een interface niet binnen 3 seconden (bijvoorbeeld omdat hij uit staat), dan stopt het programma met proberen en toont dit in de statusregel. De verbinding wordt opnieuw gemaakt zodra een decoderadres is ingevoerd (gevolgd door Return) of SELECT is gedrukt; als de verbinding al bestaat, verandert er niets.

Zoals beschreven in de Lenz-specificatie van de LAN/USB-interface 23151 (paragraaf 2.1), stopt de interface met het sturen van broadcasts (dus ook RS-Bus-terugmeldingen) naar een LAN-apparaat waarvan hij een tijd niets gehoord heeft. Daarom stuurt het programma elke 10 seconden het XpressNet-commando "Interface Status", maar alleen als er geen ander commando openstaat. Antwoordt een interface niet binnen 5 seconden, dan wordt de verbinding gesloten en wordt dit in de statusregel getoond.
