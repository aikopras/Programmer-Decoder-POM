# Programmer-Decoder-POM (MAC OS-X)
Xcode project: MAC software to configure the DCC switch decoders

This program allows the initialisation (by reading and modifying CV variables) of the [switch decoders](https://github.com/aikopras/OPENDECODER22). These decoders can be used to change the position of switches and relays. Any changes regarding the position of switches will be send back to the master station via RS-Bus  messages. Given that it uses RS-Bus messages for feedback, the decoders will primarily be interesting in environments with LENZ Master stations (like the LZV 100) connected via the LAN/USB interface (23151).<BR>
The program is written for MAC OSX; the executable can be [dowloaded directly](/Compiled%20Application/Programmer_Decoder_PoM.dmg) or can be compiled from scratch using Xcode.<BR>

## Main screen ## 
After startup the program shows its main screen. After entering the current Switch address of the decoder all CV values are being downloaded from the decoder. Screenshots for the other tabs can be seen [here](/Screenshots/).
![Main](/Screenshots/Control.png)


## How does the program work? ##
To initialise and modify the decoder's Configuration Variables (CVs) the program sends Programming on the Main (PoM) messages. Since the XPressNet specification  supports PoM messages only for train decoders (but not for accessory / feedback decoders), the "trick" the Switch decoder uses is to listen to the loco address equal to the <I>Decoder address + 6999</I>.<BR>
The requested CV values are send back via the RS-Bus.

The program communicates with the LENZ Master station via the LAN/USB interface (23151). If different master stations are used for DCC commands and RS-Bus feedback messages, two LAN/USB interfaces may be used. The IP address(es) of the LAN interfaces can be entered via the program preferences.<BR>
![Main](/Screenshots/Preferences.png)
