# Programmer-Decoder-POM (macOS)
Xcode project: macOS software to configure the DCC switch decoders

This program allows the initialisation (by reading and modifying CV variables) of the [switch decoders](https://github.com/aikopras/OPENDECODER22). These decoders can be used to change the position of switches and relays. Any changes regarding the position of switches will be send back to the master station via RS-Bus messages. Given that it uses RS-Bus messages for feedback, the decoders will primarily be interesting in environments with LENZ Master stations (like the LZV 100) connected via the LAN/USB interface (23151).<BR>
The program is written for macOS; the executable can be [downloaded directly](/Compiled%20Application/Programmer%20Decoder-POM.zip) or can be compiled from scratch using Xcode (see below).<BR>

## Requirements ##
- macOS 12 (Monterey) or later.
- Universal binary: runs natively on Apple silicon Macs and on Intel Macs.
- One or two Lenz LAN/USB interfaces (23151).

## Installation ##
1. Download [Programmer Decoder-POM.zip](/Compiled%20Application/Programmer%20Decoder-POM.zip), unzip it and move the application to the Applications folder.
2. The application is not notarized by Apple, so macOS blocks it the first time it is opened. To allow it, open System Settings → Privacy & Security (on macOS 12: System Preferences → Security & Privacy → General) and click "Open Anyway".
3. On recent macOS versions the system may ask whether the application may access devices on the local network. Allow this; otherwise the TCP connections to the Lenz interface(s) fail.

## Main screen ##
After startup the program shows its main screen. After entering the current Switch address of the decoder all CV values are being downloaded from the decoder. Screenshots for the other tabs can be seen [here](/Screenshots/).
![Main](/Screenshots/Control.png)

## How does the program work? ##
To initialise and modify the decoder's Configuration Variables (CVs) the program sends Programming on the Main (PoM) messages. Since the XPressNet specification supports PoM messages only for train decoders (but not for accessory / feedback decoders), the "trick" the Switch decoder uses is to listen to the loco address equal to the <I>Decoder address + 6999</I>.<BR>
The requested CV values are send back via the RS-Bus.

The program communicates with the LENZ Master station via the LAN/USB interface (23151). If different master stations are used for DCC commands and RS-Bus feedback messages, two LAN/USB interfaces may be used. The IP address(es) of the LAN interfaces can be entered via the program preferences.<BR>
![Main](/Screenshots/Preferences.png)

## Connections to the Lenz interface(s) ##
When the program starts, it connects to the Lenz interface(s). If an interface does not respond within 3 seconds (for example because it is switched off), the program stops trying and shows this in the status line. The connection is made again as soon as a decoder address is entered (followed by Return) or SELECT is pressed; if the connection already exists, nothing changes.

As described in the Lenz specification of the LAN/USB Interface 23151 (section 2.1), the interface stops sending broadcasts (including the RS-Bus feedback messages) to a LAN device it has not heard from for some time. Therefore the program sends the XpressNet "Interface Status" command every 10 seconds, but only when no other command is pending. If an interface does not answer within 5 seconds, the connection is closed and this is shown in the status line.

## Building from source ##
Open `Programmer Decoder-POM.xcodeproj` in Xcode 26 or later. The Release configuration (for example via Product → Build For → Profiling, or Product → Archive) produces a universal binary for Apple silicon and Intel. The help files are described in [Helpfiles/readme.txt](/Helpfiles/readme.txt).

The TCP connections are handled by `TCPSocketClass` (`TCPSocket.h` / `TCPSocket.m`), which is based on Apple's Network framework. This class contains no DCC, RS-Bus or Lenz specific code; the same class is used in [OSX-DCC-Monitor](https://github.com/aikopras/OSX-DCC-Monitor).

## Version history ##
**2.0** (October 2026)
- Updated to Xcode 26; universal binary (Apple silicon and Intel); requires macOS 12 or later.
- TCP connections via Apple's Network framework, instead of the NSStream functions that Apple will deprecate. The program no longer hangs if a Lenz interface does not respond.
- The connection is made again when a decoder address is entered or SELECT is pressed.
- XpressNet keep-alive for the Lenz interface(s) every 10 seconds.
- User interface adapted to current macOS versions.
- Several small bug fixes in the communication with the Lenz interface(s).

Earlier versions: see the commit history.
