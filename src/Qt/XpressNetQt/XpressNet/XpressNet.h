//
//  XpressNet.h
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt (C++) version of the VB6 XpressNet DLL (XpressNetClass). Version 2.0, November 2026.
//  It is a port of the .NET version (XpressNet.NET), and has the same functionality.
//  The library supports XpressNet over TCP (Lenz LAN/USB Interface 23151), over USB and over a serial interface,
//  as well as the XpressNet tunnel of the Z21 LAN protocol (UDP).
//  The library uses Qt 6 (QtCore, QtNetwork and QtSerialPort) and C++17. It is written and tested on Linux, but should
//  also be usable on macOS and Windows: it uses no Linux specific functions.
//
//  The various methods contain references to the V3.6, V4 and Z21 specifications:
//  - Befehlsbeschreibung XpressNet V3.6 mit LAN/USB Interface 23151
//    Kommunikation auf den Schnittstellen, Doku Version 1.2 05/2019
//    https://www.lenz-elektronik.de/src/pdf/23151_Info.zip
//  - Dokumentation XpressNet Version 4.0, 02/2022:
//    https://www.lenz-elektronik.de/src/pdf/Lenz_XpressNet_Doku.pdf
//  - Z21 LAN Protocol Specification, Version 1.13, 06.11.2023
//
//  Convention (as in the DLL): everything intended for the library user starts with a capital letter.
//  Internal objects, methods and attributes start with lowercase.
//  - Methods: same names and parameters as in the DLL. VB6 Integer and Long become int, Byte becomes quint8,
//    Boolean becomes bool and String becomes QString. An Optional parameter keeps its default value.
//  - Events: Qt signals with the same names and parameters as the DLL events. The application connects the
//    signals it needs (as with VB6 WithEvents). Example:
//      connect(xpressNet, &XpressNetClass::Feedback, this, &MainWindow::onFeedback);
//  - Attributes: public member variables (the attributes the library user may read are set by the library).
//  - Everything runs in the thread that created the XpressNetClass object (normally the main / GUI thread), as
//    the Qt sockets do. The signals are emitted in that thread, during the handling of the received message.
//
//  Differences with the VB6 DLL (the same as the .NET version, unless stated otherwise):
//  - The attribute ZentraleVersion is called ZentraleSoftwareVersion: in C++ a signal and an attribute can not have
//    the same name; the signal ZentraleVersion keeps its name.
//  - CmdConnectUsb and CmdConnectSerial: Port is the name of the serial device, for example /dev/ttyUSB0 (FTDI, such
//    as the LI-USB) or /dev/ttyACM0 (USB of the chip itself) on Linux, COM3 on Windows (VB6: a COM port number).
//    If the port can not be opened, the event TcpError is raised, as in the macOS version (VB6 and .NET: a MessageBox;
//    the library has no graphical user interface).
//  - Opening a new connection first closes an existing connection.
//  - As long as the connection is not (yet) established, messages are dropped (event SendError KeineVerbindung).
//  - The destructor (delete) closes the connection and stops the timers (VB6: Set XpressNet = Nothing).
//  - TcpClosedByRemoteHost: the connection is cleaned up before the event is raised (VB6: after), so the event
//    handler of the application may open a new connection.
//  - A parameter value that does not fit in a byte is not sent, but written to the logfile (VB6: run time error
//    "Overflow").
//  - The objects and methods for internal use (send, receive, tcp, eventFeedback, ...) are not meant to be used by the
//    application. They are public, since the classes of the library use them.
//
//  New in version 2.0 (November 2026), the same in the VB6, .NET, macOS and Qt versions:
//  - Retransmission of a message after an error answer of the interface, and a pause as long as the command
//    station does not address the interface (01 05 / 01 07). See XpressNetSend.cpp
//  - The events SendError, InterfaceNichtAdressiert and InterfaceWiederAdressiert. SendError is raised for every
//    message that is not sent (not silently dropped). The reasons are in SendErrorReasonEnum
//  - CmdConnectUsb has the optional parameter Handshake (hardware handshake RTS/CTS, default off)
//  - The names of more command stations in the logfile (LokMaus II, LDH, multiMAUS)
//  - The default directory of the logfile is ~/TMC-Logfiles (macOS: ~/Library/Logs/TMC-Logfiles, Windows:
//    %PUBLIC%\TMC-Logfiles; see XpressNetLogFile.cpp)
//  - In the logfile every line starts with the letter of the XpressNet object (A, B, ...), and the description of a
//    message with => (sent) or <= (received)
//  - CmdAccessoryAuto switches off the activated switch if the queue is emptied (see XpressNetAccAuto.cpp)
//  - After a received message with an XOR error, the next framing bytes are searched (see XpressNetReceive.cpp)
//  - Z21: CmdInitUdp first sets the broadcast flags (LAN_SET_BROADCASTFLAGS); new command CmdZ21SetBroadcastFlags.
//    A new connection and the destructor first send LAN_LOGOFF
//  - TCP: a connection attempt is stopped after 5 seconds
//

#ifndef XPRESSNET_H
#define XPRESSNET_H

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <initializer_list>

class SendClass;
class ReceiveClass;
class ReceiveClassZ21;
class AccAutoClass;
class LogFileClass;
class TCPSocketClass;
class UDPSocketClass;
class SerialPortClass;


// Enumeration to determine the type of connection
enum connectionTypeEnum
{
  none,
  lanTcpPort,
  lanUdpPort,
  usbPort,
  serialPort
};

// 2.0: The values of the parameter Reason of the event SendError. The description (parameter Description) contains
// the bytes of the message (including the XOR byte, without the framing bytes)
// The names are those of the Lenz specification (Interface 23151, V1.2), where the specification has a name
enum SendErrorReasonEnum : quint8
{
  WiederholungErfolglos = 1,                    // Given up after 3 retransmissions (61 80, 61 81, 01 01, 01 02, 01 03,
                                                // 01 06, 01 0A). Not a name of the specification
  BefehlNichtVorhanden = 2,                     // Answer 61 82 (V3.6: 3.1.7)
  Timeout = 3,                                  // No answer within 5 sec (90 sec in programming mode; V3.6: 1.3).
                                                // All queues are emptied
  FifoVoll = 4,                                 // The FIFO (128 messages) or the CmdAccessoryAuto queue (32 commands) is
                                                // full. Not a name of the specification
  KeineVerbindung = 5,                          // There is no connection, or the connection has not (yet) been
                                                // established. Not a name of the specification
  FehlerInDenBefehlsparametern = 6,             // Answer 01 09 (V3.6: 1.5), or a parameter value out of range (checked
                                                // by the library; the message is not sent)
  ZentraleAdressiertDasInterfaceNichtMehr = 7   // No 01 07 within 5 sec (90 sec in programming mode) after 01 05 / 01 08
                                                // (V3.6: 1.5). All queues are emptied
};


class XpressNetClass : public QObject
{
  Q_OBJECT

  // ****************************************************************************************************************
  // *************************************************** EVENTS *****************************************************
  // ****************************************************************************************************************
signals:
  // GENERATED EVENTS - Interface: TCP as well as USB
  void InterfaceConnected();
  void TcpClosedByRemoteHost();
  void TcpError(QString Description);
  // GENERATED EVENTS - USB
  void UsbConnected();                                    // As in the VB6 DLL: declared, but never raised
  // GENERATED EVENTS - XPRESSNET
  void AllesAn();
  void NotAus();
  void NotHalt();
  void ProgrammierMode();
  void XpressNetAddress(quint8 address);
  void XpressNetVersion(QString Version);
  void InterfaceVersion(QString Version, QString code);
  void InterfaceFreieVerbindungen(quint8 number);
  void ZentraleVersion(QString Version);
  void ErweiterteZentraleVersion(QString BuildZentrale, QString VersionRuckMelder, QString BuildRuckMelder, QString VersionBootloader);
  void Fehlermeldung(quint8 value);
  void ServiceVariable(quint8 SvAddress, quint8 SvValue);
  // Feedback events
  void Feedback(int address, quint8 Nibble, quint8 data);
  void SwitchFeedback(int address, quint8 ttBits, quint8 iBit, quint8 data);
  void ExtendedAccessoryInfo(int address, quint8 value, quint8 Status);
  // Lok events
  void NormaleLokinfo(bool Besetzt, int Speedsteps, int Speed, bool Forward, quint8 F0_F4, quint8 F5_F8, quint8 F9_F12);
  void FunctionsZustandF13F20(quint8 F13_F20, quint8 F21_F28);
  void FunctionsZustandF29F68(quint8 F29_F36, quint8 F37_F44, quint8 F45_F52, quint8 F53_F60, quint8 F61_F68);
  void FunctionStatusF0F12(quint8 F0_F4, quint8 F5_F8, quint8 F9_F12);
  void FunctionStatusF13F20(quint8 F13_F20, quint8 F21_F28, quint8 RefreshMode);
  void FunctionStatusF29F68(quint8 F29_F36, quint8 F37_F44, quint8 F45_F52, quint8 F53_F60, quint8 F61_F68);
  // Programmier events - Service Mode
  void ProgrammierinfoKurzschluss();
  void ProgrammierinfoDatenNichtGefunden();
  void ProgrammierinfoZentraleBusy();
  void ProgrammierinfoZentraleBereit();
  void ProgrammierinfoCV(int CV, quint8 value);
  void ProgrammierinfoPageRegister(quint8 EEPromAddress, quint8 value);
  // Programmier events - Programming on Main (PoM)
  void PomErgebnis(int LokAddress, quint8 CvValue);
  // Modellzeit events
  void Modellzeit(quint8 day, quint8 hour, quint8 minute, quint8 factor);
  // Z21 specific events
  void z21FirmwareVersion(QString Version);
  void Z21Lokinfo(int address, bool Besetzt, int Speedsteps, int Speed, bool Forward, quint8 F0_F4, quint8 F5_F8, quint8 F9_F12, quint8 F13_F20, quint8 F21_F28);
  // 2.0: Sending (see XpressNetSend.cpp)
  void SendError(quint8 Reason, QString Description);
  void InterfaceNichtAdressiert();
  void InterfaceWiederAdressiert();


  // ****************************************************************************************************************
  // ************************************************* ATTRIBUTES ***************************************************
  // ****************************************************************************************************************
public:
  // ATTRIBUTES THAT THE LIBRARY USER MAY CHANGE. THE DEFAULT VALUES ARE SET IN THE CONSTRUCTOR
  bool Z21Protocol;                                       // Is the library used for Z21 or XpressNet??
  bool LogMessages;                                       // Example: connection requested to host X
  bool LogWarnings;                                       // May indicate a software problem
  bool LogErrors;                                         // Indicates a hardware or software problem
  bool LogHexMessages;                                    // Hex dump of transmitted / received messages
  bool LogDetails;                                        // Description of what has been transmitted / received
  int AccessoryInterval;                                  // Interval (ms) between two accesory commands

  // ATTRIBUTES THAT THE LIBRARY USER MAY READ
  bool ZentraleNotAus;
  bool ZentraleNotHalt;
  bool ZentraleProgrammierMode;
  QString ZentraleSoftwareVersion;                        // VB6: ZentraleVersion (see the header of this file)
  quint8 ZentraleKennung;
  bool ZentraleKurzSchluss;                               // Only for Z21
  bool ZentraleStartmodeAuto;
  bool ZentraleKaltstart;
  bool ZentraleRamCheckFehler;
  int NumberOfExpressNetMessagesReceived;
  int NumberOfExpressNetMessagesTransmitted;

  // OBJECTS THAT THE LIBRARY USER MAY USE
  LogFileClass *Logfile;

  // The type of connection
  connectionTypeEnum connectionType;

  // Objects for internal use. Although these are public, they should not be used outside the library
  SendClass *send;                                        // Everything related to sending XpressNet messages
  ReceiveClass *receive;                                  // Everything related to receiving XpressNet messages
  ReceiveClassZ21 *receiveZ21;                            // To receive messages transported over a Z21 XpressNet tunnel
  TCPSocketClass *tcp;                                    // The TCP connection (Lenz LAN interface)
  UDPSocketClass *udp;                                    // The UDP connection (Z21)
  SerialPortClass *usb;                                   // The USB or serial connection
  AccAutoClass *AccAuto;                                  // Implements the AccessoryAuto commands
  QString instanceId;                                     // 2.0: the letter of this object in the logfile (A, B, ...)

  // 2.0: Z21: the broadcast flags that CmdInitUdp sets (see CmdZ21SetBroadcastFlags)
  static const int Z21BroadcastFlagsDefault = 0x00000001;


  // ****************************************************************************************************************
  // ********************************************** CONSTRUCTOR AND DESTRUCTOR **************************************
  // ****************************************************************************************************************
  explicit XpressNetClass(QObject *parent = nullptr);   // VB6: Class_Initialize
  ~XpressNetClass() override;                             // VB6: Class_Terminate (Set XpressNet = Nothing)


  // ****************************************************************************************************************
  // ********************************************* METHODS FOR ESTABLISHING CONNECTIONS *****************************
  // ****************************************************************************************************************
  void CmdConnectTcp(const QString &Host, const QString &Port = "5550");
  void CmdInitUdp(const QString &Host, const QString &Port = "21106");
  void CmdConnectUsb(const QString &Port, const QString &Settings = "115200,N,8,1", bool Handshake = false);   // Lenz: 57600
  void CmdConnectSerial(const QString &Port, const QString &Settings, bool Handshake);

  // ****************************************************************************************************************
  // ********************************************* METHODS TO GENERATE INTERFACE SPECIFIC MESSAGES ******************
  // ****************************************************************************************************************
  void CmdInterfaceVersion();                             // V3.6: 1.6
  void CmdXpressNetAddress(quint8 address = 0);           // V3.6: 1.7
  void CmdInterfaceStatus();                              // V3.6: 2.1 (keep alive)
  void CmdXpressNetVersion();                             // V3.6: 2.2
  void CmdInterfaceFreieVerbindungen();                   // V3.6: 2.3

  // ****************************************************************************************************************
  // ******************************** V3.6 & V4 METHODS TO GENERATE COMMAND STATION SPECIFIC MESSAGES ***************
  // ****************************************************************************************************************
  void CmdAllesAn();
  void CmdAllesAus();
  void CmdAlleLoksAnhalten();
  void CmdEineLokAnhalten(int address);
  void CmdLeseanfrageProgrammierenRegistermode(quint8 Register);
  void CmdLeseanfrageProgrammieren(int CV);
  void CmdLeseanfrageProgrammierenPagemode(int CV);
  void CmdProgrammierergebnisAnfordern();
  void CmdSchreibbefehlProgrammierenRegisterMode(quint8 Register, int daten);
  void CmdSchreibbefehlProgrammieren(int CV, quint8 Val);
  void CmdSchreibbefehlProgrammierenPageMode(int CV, int daten);
  void CmdSoftwareversionZentraleAnfordern();
  void CmdStatusZentraleAnfordern();
  void CmdZentralenStartmodeSetzen(bool AutoStart);
  void CmdSwitchInfoRequest(int address);                 // Intended for switches
  void CmdFeedbackRequest(int address, quint8 Nibble);    // Intended for RS-Bus decoders
  void CmdAccessory(int address, quint8 position, quint8 Activate);
  void CmdAccessoryAuto(int Switch, quint8 position);
  void CmdLokInfoAnfordern(int address);
  void CmdFunktionsStatusAnfordern(int address);
  void CmdFunktionsStatusF13F28Anfordern(int address);
  void CmdFunktionsZustandF13F28Anfordern(int address);
  void CmdFunktionsStatusF29F68Anfordern(int address);
  void CmdFunktionsZustandF29F68Anfordern(int address);
  void CmdFahrbefehl(int address, int Speed, int Steps, bool Forward);
  void CmdLokFunktionBefehl(int address, quint8 group, quint8 data);
  void CmdLokFunktionStatus(int address, quint8 group, quint8 data);
  void CmdPomWrite(int address, int CV, quint8 value);
  void CmdPomRead(int address, int CV);
  void CmdPomBitWrite(int address, int CV, bool bitValue, quint8 bitPosition);
  void CmdAdresAnfrageLokInStack(int address, bool Vorwarts);
  void CmdLokAusStack(int address);

  // ****************************************************************************************************************
  // ******************************** V4 ONLY METHODS TO GENERATE COMMAND STATION SPECIFIC MESSAGES *****************
  // ****************************************************************************************************************
  void CmdErweiterteZentralenVersionsinformation();
  void CmdPoMErgebnisAnfordern();
  void CmdZentralenResetAuslosen();
  void CmdModellzeitAnfordern();
  void CmdModellzeitStellen(quint8 day, quint8 hour, quint8 minute, quint8 factor);
  void CmdModellzeitAnhalten();
  void CmdModellzeitStarten();

  // ****************************************************************************************************************
  // ******************************** Z21 ONLY METHODS TO GENERATE COMMAND STATION SPECIFIC MESSAGES ****************
  // ****************************************************************************************************************
  void CmdZ21SetBroadcastFlags(int Flags);                // 2.0
  void CmdZ21GetFirmwareVersion();
  void CmdZ21SetLocoFunction(int address, quint8 CV, quint8 TT);
  void CmdZ21SetExtAccessory(int address, quint8 value);
  void CmdZ21GetExtAccessory(int address);
  void CmdZ21AccPomWrite(int decoderAddress, quint8 CDDD, int CV, quint8 value);
  void CmdZ21AccPomBitWrite(int decoderAddress, quint8 CDDD, int CV, bool bitValue, quint8 bitPosition);
  void CmdZ21AccPomRead(int decoderAddress, quint8 CDDD, int CV);
  void CmdZ21MmWriteByte(quint8 RegAdr, quint8 value);

  // ****************************************************************************************************************
  // ******************************** OpenDCC ONLY METHODS TO GENERATE COMMAND STATION SPECIFIC MESSAGES ************
  // ****************************************************************************************************************
  void CmdOpenDCCSetExtAccessory(int address, quint8 value);

  // Derive from the speed byte an integer value (for decoders with 27/28 speed steps)
  int DecodeSpeed(int Speed);


  // ****************************************************************************************************************
  // ************************** RAISING EVENTS: SHOULD NOT BE CALLED BY THE LIBRARY USER! ***************************
  // ****************************************************************************************************************
  // The methods below forward events from lower level objects to the library user.
  // In addition Logfile messages are written, if needed.
  void eventInterfaceConnected();
  void eventTcpError(const QString &Description, const QString &logText = QString());
  void eventTcpClosedByRemoteHost();
  void eventInterfaceVersion(quint8 daten1, quint8 daten2);
  void eventXpressNetAddress(quint8 address);
  void eventXpressNetVersion(quint8 daten);
  void eventInterfaceFreieVerbindungen(quint8 number);
  void eventAllesAn();
  void eventNotAus();
  void eventNotHalt();
  void eventProgrammierMode();
  void eventFeedback(int address, quint8 Nibble, quint8 ttBits, quint8 iBit, quint8 data);
  void eventProgrammierinfoKurzschluss();
  void eventProgrammierinfoDatenNichtGefunden();
  void eventProgrammierinfoZentraleBusy();
  void eventProgrammierinfoZentraleBereit();
  void eventProgInfoPageRegister(quint8 EEPromAddress, quint8 value);
  void eventProgInfo(quint8 CvRange, quint8 daten1, quint8 value);
  void eventProgInfoZ21(quint8 CVAdr_MSB, quint8 CVAdr_LSB, quint8 value);
  void eventServiceVariableMelden(quint8 SvAddress, quint8 SvValue);
  void eventSoftwareVersionZentrale(quint8 daten2, quint8 daten3);
  void eventStatusZentrale(quint8 data);
  void eventPomErgebnisMelden(quint8 AdrH, quint8 AdrL, quint8 CvValue);
  void eventNormaleLokinfo(quint8 Kennung, quint8 Speed, quint8 F0_F4, quint8 F5_F12);
  void eventZ21Lokinfo(quint8 Adr_MSB, quint8 Adr_LSB, quint8 Kennung, quint8 Speed, quint8 F0_F4, quint8 F5_F12, quint8 F13_F20, quint8 F21_F28);
  void eventFunctionsZustandF13F20(quint8 F13_F20, quint8 F21_F28);
  void eventFunctionsZustandF29F68(quint8 F29_F36, quint8 F37_F44, quint8 F45_F52, quint8 F53_F60, quint8 F61_F68);
  void eventFunctionStatusF0F12(quint8 F0_F4, quint8 F5_F12);
  void eventFunctionStatusF13F20(quint8 F13_F20, quint8 F21_F28, quint8 RefreshMode);
  void eventFunctionStatusF29F68(quint8 F29_F36, quint8 F37_F44, quint8 F45_F52, quint8 F53_F60, quint8 F61_F68);
  void eventFehlermeldung(quint8 data);
  void eventModellZeit(quint8 daten1, quint8 daten2, quint8 factor = 255);
  void eventErweiterteVersionZentrale(quint8 ZBldH, quint8 ZBldL, quint8 RMVer, quint8 RMBldH, quint8 RMBldL, quint8 BIvers);
  void eventz21FirmwareVersion(quint8 V_MSB, quint8 V_LSB);
  void eventExtendedAccessoryInfo(quint8 Adr_MSB, quint8 Adr_LSB, quint8 DDDDDDDD, quint8 Status);
  void eventSendError(SendErrorReasonEnum reason, const QString &description);   // 2.0
  void eventInterfaceNichtAdressiert();                   // 2.0
  void eventInterfaceWiederAdressiert();                  // 2.0

  // ****************************************************************************************************************
  // ****************************** Methods for internal library use. Should not be called by the library user *****
  // ****************************************************************************************************************
  bool isConnected();                                     // 2.0: do we have a connection on which we can send?
  static double Val(const QString &text);                 // The VB6 Val function (see XpressNet.cpp)

private:
  void closeConnections();
  bool VelueOutOfRange(int value, int min, int max, const QString &LogText);
  int AccAL(int address);
  int AccAH(int address);
  static int AL(int address);
  static int AH(int address);
  static int EncodeSpeed(int Speed);
  int DecodeSpeedSteps(int Speed, int Speedsteps);
  static bool bitValue(int data, int index);
  static QString zentraleName(quint8 kennung);
  static QString letters(int n);
  void requestZentraleInfo();
  static int instanceCounter;

  // Known values for the Zentralen-Kennung
  static const quint8 LZ100 = 0x00;                       // LZ 100
  static const quint8 LH200 = 0x01;                       // LH 200
  static const quint8 DPC = 0x02;                         // DPC - Zentrale (Compact und Commander)
  static const quint8 CntrPlus = 0x03;                    // Control Plus - Zentrale
  static const quint8 LokMausII = 0x04;                   // 2.0: Roco LokMaus II (source: JMRI XNetConstants)
  static const quint8 LDH = 0x07;                         // 2.0: LDH (source: JMRI XNetConstants)
  static const quint8 multiMAUS = 0x10;                   // 2.0: Roco multiMAUS (source: JMRI XNetConstants)
  static const quint8 Z21 = 0x12;                         // Z21 / Yamorc
  static const quint8 OpenDcc = 0x20;                     // See: https://www.opendcc.de/elektronik/opendcc/xpressnet_commands_e.html
  static const quint8 OpenDccXp = 0x22;                   // See: https://www.opendcc.de/elektronik/opendcc/xpressnet_commands_e.html
};

#endif // XPRESSNET_H
