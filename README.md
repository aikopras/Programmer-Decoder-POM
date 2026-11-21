[🇬🇧 English](README.md) | [🇩🇪 Deutsch](README.de.md) | [🇳🇱 Nederlands](README.nl.md)

# Programmer-Decoder-POM
Software to configure the DCC accessory decoders with RS-Bus feedback, including the switch, relays, lift and servo decoders.

This program allows the initialisation (by reading and modifying CV variables) of the [switch decoders](https://github.com/aikopras/OPENDECODER22) and related decoders that are based on the [AP_DCC_Decoder_Core](https://github.com/aikopras/AP_DCC_Decoder_Core) library. These decoders are used to change the position of switches, relays and servos. Any changes regarding the position of switches and servos are sent back to the master station via RS-Bus messages. Given that the decoders use RS-Bus messages for feedback, the decoders will primarily be interesting in environments with LENZ Master stations (like the LZV 100) connected via a LAN/USB interface (such as the 23151).

## Getting the value of CVs ##
To initialise and modify the decoder's Configuration Variables (CVs) the program sends Programming on the Main (PoM) messages. Since the XpressNet specification supports PoM messages only for train decoders (but not for accessory / feedback decoders), the "trick" the decoders use is to listen to the loco address equal to the <I>Decoder address + 6999</I>. The requested CV values are sent back via the RS-Bus, using address 128. Since all decoders share this address for PoM feedback messages,  decoders must be configured one by one.

The program communicates with the LENZ Master station via the LAN/USB interface (23151). If different master stations are used for DCC commands and RS-Bus feedback messages, two LAN/USB interfaces may be used.

## Startup screen
The program starts with a screen to enter the switch address, and show some basic information regarding this decoder. The various program tabs are further explained in the [Manual](Manual/Manual.md).
![Main](Manual/Screenshots/Control.png)

## Download
| Version | Runs on | Program and installation | Source code and building |
|---|---|---|---|
| macOS | macOS 12 or later, Apple silicon and Intel | [Compiled-Applications/macOS](Compiled-Applications/macOS/README.md) | [src/Xcode](src/Xcode/README.md) (Objective-C) |
| Windows | Windows 10 / 11 (64 bit), Intel / AMD and ARM | [Compiled-Applications/Windows](Compiled-Applications/Windows/README.md) | [src/Qt](src/Qt/README.md) (C++ / Qt 6) |
| Linux | x86_64 | [Compiled-Applications/Linux](Compiled-Applications/Linux/README.md) | [src/Qt](src/Qt/README.md) (C++ / Qt 6) |

The changes per version are described in the [Version history](Version_History.md).
