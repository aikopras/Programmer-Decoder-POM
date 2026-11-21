[🇬🇧 English](README.md) | [🇩🇪 Deutsch](README.de.md) | [🇳🇱 Nederlands](README.nl.md)

# Programmer-Decoder-POM
Software om de DCC-accessorydecoders met RS-Bus-terugmelding in te stellen, waaronder de wissel-, relais-, lift- en servodecoders.

Met dit programma kunnen de [wisseldecoders](https://github.com/aikopras/OPENDECODER22) en verwante decoders die gebaseerd zijn op de library [AP_DCC_Decoder_Core](https://github.com/aikopras/AP_DCC_Decoder_Core) ingesteld worden, door hun CV's te lezen en te wijzigen. Met deze decoders worden wissels, relais en servo's omgezet. Elke wijziging in de stand van wissels en servo's wordt via RS-Bus-berichten teruggemeld aan de centrale. Omdat de decoders de RS-Bus voor de terugmelding gebruiken, zijn ze vooral interessant voor banen met LENZ-centrales (zoals de LZV 100) die via een LAN/USB-interface (zoals de 23151) zijn aangesloten.

## CV-waarden uitlezen ##
Om de CV's van de decoder in te stellen en te wijzigen, stuurt het programma PoM-berichten (Programming on the Main). Omdat de XpressNet-specificatie PoM-berichten alleen voor locdecoders kent (niet voor accessory- of terugmelddecoders), luisteren de decoders als "truc" naar het locadres <I>decoderadres + 6999</I>. De gevraagde CV-waarden worden via de RS-Bus teruggemeld, met adres 128. Omdat alle decoders dit adres delen voor hun PoM-terugmeldingen, moeten de decoders één voor één worden ingesteld.

Het programma communiceert met de LENZ-centrale via de LAN/USB-interface (23151). Worden voor DCC-commando's en RS-Bus-terugmeldingen verschillende centrales gebruikt, dan kunnen twee LAN/USB-interfaces worden gebruikt.

## Startscherm ##
Het programma start met een scherm waarin het wisseladres wordt ingevoerd en enkele gegevens van deze decoder worden getoond. De tabs van het programma worden verder uitgelegd in de [handleiding](Manual/Manual.nl.md).
![Hoofdscherm](Manual/Screenshots/Control.png)

## Download ##
| Versie | Draait op | Programma en installatie | Broncode en bouwen |
|---|---|---|---|
| macOS | macOS 12 of nieuwer, Apple silicon en Intel | [Compiled-Applications/macOS](Compiled-Applications/macOS/README.md) | [src/Xcode](src/Xcode/README.md) (Objective-C) |
| Windows | Windows 10 / 11 (64 bit), Intel / AMD en ARM | [Compiled-Applications/Windows](Compiled-Applications/Windows/README.md) | [src/Qt](src/Qt/README.md) (C++ / Qt 6) |
| Linux | x86_64 | [Compiled-Applications/Linux](Compiled-Applications/Linux/README.md) | [src/Qt](src/Qt/README.md) (C++ / Qt 6) |

De wijzigingen per versie staan in de [Version history](Version_History.md) (Engels).
