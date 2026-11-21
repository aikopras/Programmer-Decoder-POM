//
//  XpressNet.cpp
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of the VB6 XpressNet DLL (XpressNetClass). Version 2.0, November 2026. See XpressNet.h.
//  A port of XpressNet.cs of the .NET version.
//

#include "XpressNet.h"
#include "XpressNetSend.h"
#include "XpressNetReceive.h"
#include "XpressNetReceiveZ21.h"
#include "XpressNetAccAuto.h"
#include "XpressNetLogFile.h"
#include "TCPSocket.h"
#include "UDPSocket.h"
#include "SerialPort.h"
#include <QThread>

// Number to text, and Boolean to text as in VB6 and .NET ("True" / "False")
static inline QString str(int value) { return QString::number(value); }
static inline QString tf(bool value) { return value ? QStringLiteral("True") : QStringLiteral("False"); }

int XpressNetClass::instanceCounter = 0;


// ******************************************************************************************************************
// ******************************************* CONSTRUCTOR AND DESTRUCTOR *******************************************
// ******************************************************************************************************************
// 2.0: the letter of this object in the logfile: A for the first XpressNet object of the program, B for the second,
// and so on (after Z: AA, AB, ...). Two programs that write in the same logfile both start with A
QString XpressNetClass::letters(int n)
{
  QString s;
  while (n > 0) {
    n = n - 1;
    s.prepend(QChar('A' + (n % 26)));
    n = n / 26;
  }
  return s;
}

// VB6: Class_Initialize
XpressNetClass::XpressNetClass(QObject *parent) : QObject(parent)
{
  instanceCounter = instanceCounter + 1;
  instanceId = letters(instanceCounter);
  connectionType = none;                                  // We start without any connection
  tcp = nullptr;
  udp = nullptr;
  usb = nullptr;
  // Logfile specific initialization. Logfile is created first, since the other objects may use it
  Logfile = new LogFileClass(this);
  send = new SendClass(this);
  receive = new ReceiveClass(this);
  receiveZ21 = new ReceiveClassZ21(this);
  AccAuto = new AccAutoClass(this);
  LogMessages = true;                                     // Set the defaults for what messages will be logged.
  LogErrors = true;
  LogWarnings = true;
  LogHexMessages = true;
  LogDetails = true;
  Logfile->OpenLogfile();                                 // Check Logfile size, and rotate if needed
  // Attributes the library user may change
  Z21Protocol = false;
  AccessoryInterval = 125;                                // Time in msec between two accessory commands
  // Attributes the library user may read
  ZentraleNotAus = false;
  ZentraleNotHalt = false;
  ZentraleKurzSchluss = false;                            // Only for Z21
  ZentraleProgrammierMode = false;
  ZentraleSoftwareVersion = "0.0";                        // Give initial version, to avoid problems if interface is not open
  ZentraleKennung = 0;
  ZentraleStartmodeAuto = false;
  ZentraleKaltstart = false;
  ZentraleRamCheckFehler = false;
  NumberOfExpressNetMessagesReceived = 0;
  NumberOfExpressNetMessagesTransmitted = 0;
}

// VB6: Class_Terminate (Set XpressNet = Nothing)
// Closes the connection and stops the timers
XpressNetClass::~XpressNetClass()
{
  Logfile->Details("Close XpressNet interface");
  closeConnections();                                     // Before leaving, make sure we close any open connections
  delete AccAuto;                                         // Stops the timer
  delete send;                                            // Stops the timers
  delete receiveZ21;
  delete receive;
  delete Logfile;
}


// ==================================================================================================================
// METHODS FOR ESTABLSHING CONNECTIONS
// Note: opening a new connection first closes an existing connection
void XpressNetClass::CmdConnectTcp(const QString &Host, const QString &Port)
{
  closeConnections();
  Logfile->myHost = Host;                                 // Save the host (IP address) to include in the Logfile
  Logfile->Details("TCP connection requested to Host: " + Host + " at Port: " + Port);
  connectionType = lanTcpPort;                            // We connect via TCP over a LAN
  tcp = new TCPSocketClass(this);                         // Instantiate a new TCP class object
  tcp->connect(Host, Port);                               // Send a connection request
}

void XpressNetClass::CmdInitUdp(const QString &Host, const QString &Port)
{
  closeConnections();
  Logfile->myHost = Host;                                 // Save the host (IP address) to include in the Logfile
  Logfile->Details("UDP initialised: Host: " + Host + ", Port: " + Port);
  connectionType = lanUdpPort;                            // We connect via UDP over a LAN
  udp = new UDPSocketClass(this);                         // Instantiate a new UDP class object
  Z21Protocol = true;
  udp->init(Host, Port);                                  // Store the address and port
  // 2.0: the first message is LAN_SET_BROADCASTFLAGS (Z21 spec 2.16). Without these flags the Z21 sends no
  // broadcasts, such as LAN_X_BC_TRACK_POWER_ON/OFF (events AllesAn / NotAus) and LAN_X_TURNOUT_INFO.
  // The flags must be set again after every login; the login is done with the first command (Z21 spec 2.2)
  CmdZ21SetBroadcastFlags(Z21BroadcastFlagsDefault);
  CmdSoftwareversionZentraleAnfordern();
  CmdStatusZentraleAnfordern();
  CmdZ21GetFirmwareVersion();
}

// 2.0: the optional parameter Handshake (default: off). The Lenz documentation of the LI-USB and the 23151 says
// no handshake; JMRI uses hardware handshake (RTS/CTS) for the LI-USB 23150. See SerialPort.cpp
void XpressNetClass::CmdConnectUsb(const QString &Port, const QString &Settings, bool Handshake)
{
  // Lenz uses 57600, Yamorc 115200
  closeConnections();
  Logfile->myHost = Port;                                 // Save the port to include in the Logfile
  Logfile->Details("USB connect attempt using Port=" + Port + ", Settings=" + Settings + " and Handshake=" + tf(Handshake));
  connectionType = usbPort;                               // We connect via USB
  usb = new SerialPortClass(this);
  usb->openPort(Port, Settings, Handshake);               // Send a connection request
}

void XpressNetClass::CmdConnectSerial(const QString &Port, const QString &Settings, bool Handshake)
{
  closeConnections();
  Logfile->myHost = Port;                                 // Save the port to include in the Logfile
  Logfile->Details("Serial connect attempt using Port=" + Port + ", Settings=" + Settings + " and Handshake=" + tf(Handshake));
  connectionType = serialPort;                            // We connect via a serial (RS232) connection
  usb = new SerialPortClass(this);                        // Note that usb and serial use the same class and object
  usb->openPort(Port, Settings, Handshake);               // Send a connection request
}


// ==================================================================================================================
// METHODS TO GENERATE INTERFACE SPECIFIC MESSAGES
// The various methods below contain references to the V3.6 and V4 XpressNet specifications. For details, see these
// documents (see the header of XpressNet.h)

// V3.6: 1.6 (Versionsnummer des Interface 23151 feststellen)
void XpressNetClass::CmdInterfaceVersion()
{
  send->message({0xF0});
  Logfile->DetailsSent("Versionsnummer des Interface 23151 feststellen");
}

// V3.6: 1.7 (XpressNet Gerateadresse des Interface 23151 feststellen und andern)
void XpressNetClass::CmdXpressNetAddress(quint8 address)
{
  send->message({0xF2, 0x01, address});
  Logfile->DetailsSent("XpressNet Gerateadresse des Interface 23151 feststellen");
}

// V3.6: 2.1 (Interface Status Befehl - Keep Alive) - Need not be used by the Application
void XpressNetClass::CmdInterfaceStatus()
{
  send->message({0xF1, 0x01});
}

// V3.6: 2.2 (XpressNet Version Befehl)
void XpressNetClass::CmdXpressNetVersion()
{
  send->message({0xF1, 0x02});
  Logfile->DetailsSent("XpressNet Version Befehl");
}

// V3.6: 2.3 (Verfugbare Freie Verbindungen)
void XpressNetClass::CmdInterfaceFreieVerbindungen()
{
  send->message({0xF1, 0x03});
  Logfile->DetailsSent("Verfugbare Freie Verbindungen");
}


// ==================================================================================================================
// V3.6 & V4 METHODS TO GENERATE COMMAND STATION SPECIFIC MESSAGES
// The commands below will be forwarded by the Interface to the Command Station

// V3.6: 3.2.1 / V4: 3.2 (Alles An)
void XpressNetClass::CmdAllesAn()
{
  send->message({0x21, 0x81});
  Logfile->DetailsSent("Alles An");
}

// V3.6: 3.2.2 / V4: 3.3 (Alles Aus - Notaus)
void XpressNetClass::CmdAllesAus()
{
  send->message({0x21, 0x80});
  Logfile->DetailsSent("Alles Aus - Notaus");
}

// V3.6: 3.2.3 / V4: 3.4 (Alle Loks anhalten - Nothalt)
void XpressNetClass::CmdAlleLoksAnhalten()
{
  send->message({0x80});
  Logfile->DetailsSent("Alle Loks anhalten - Nothalt");
}

// V3.6: 3.2.4 / V4: 3.6 & 3.7 (Eine Lok anhalten - Nothalt fur eine Lok)
void XpressNetClass::CmdEineLokAnhalten(int address)
{
  send->message({0x92, AH(address), AL(address)});
  Logfile->DetailsSent("Eine lok anhalten. Adresse: " + str(address));
}

// V3.6: 3.2.5 / V4: 3.9
void XpressNetClass::CmdLeseanfrageProgrammierenRegistermode(quint8 Register)
{
  if (VelueOutOfRange(Register, 1, 8, "Leseanfrage Programmieren Registermode: Register")) return;
  send->message({0x22, 0x11, Register});
  Logfile->DetailsSent("Leseanfrage Programmieren Registermode. Register: " + str(Register));
}

// V3.6: 3.2.6-3.2.10 / V4: 3.10-3.14 (Leseanfrage Programmieren 4-Byte-Format / LAN_X_CV_READ )
void XpressNetClass::CmdLeseanfrageProgrammieren(int CV)
{
  if (VelueOutOfRange(CV, 1, 1024, "Leseanfrage Programmieren: CV")) return;
  if (Z21Protocol) {
    CV = CV - 1;
    send->message({0x23, 0x11, (CV / 256), (CV % 256)});
    Logfile->DetailsSent("LAN_X_CV_READ, CV: " + str(CV + 1));
  }
  else {
    // The version is compared as a number (see Val)
    if (Val(ZentraleSoftwareVersion) < 3.6) {
      send->message({0x22, 0x15, (CV % 256)});
    }
    else {
      int daten1 = 0x18 + ((CV / 256) % 4);
      send->message({0x22, daten1, (CV % 256)});
    }
    Logfile->DetailsSent("Leseanfrage Programmieren 4-Byte-Format. CV: " + str(CV));
  }
}

// V3.6: 3.2.11 / V4: 3.15 (Leseanfrage Programmieren 3-Byte-Format - Pagemode)
void XpressNetClass::CmdLeseanfrageProgrammierenPagemode(int CV)
{
  if (VelueOutOfRange(CV, 1, 256, "Leseanfrage Programmieren Pagemode: CV")) return;
  send->message({0x22, 0x14, (CV % 256)});
  Logfile->DetailsSent("Leseanfrage Programmieren Pagemode. CV: " + str(CV));
}

// V3.6: 3.2.12 / V4: 3.16
void XpressNetClass::CmdProgrammierergebnisAnfordern()
{
  send->message({0x21, 0x10});
  Logfile->DetailsSent("Programmier Ergebnis Anfordern");
}

// V3.6: 3.2.13 / V4: 3.17 (Schreibbefehl Programmieren 3-Byte-Format (Register-Mode) / LAN_X_DCC_WRITE_REGISTER)
void XpressNetClass::CmdSchreibbefehlProgrammierenRegisterMode(quint8 Register, int daten)
{
  if (VelueOutOfRange(Register, 1, 8, "Schreibbefehl Programmieren Registermode: Register")) return;
  send->message({0x23, 0x12, Register, daten});
  Logfile->DetailsSent("Schreibbefehl Programmieren RegisterMode. Register: " + str(Register) + ", Value: " + str(daten));
}

// V3.6: 3.2.14-3.2.18 / V4: 3.18-3.22 (Schreibbefehl Programmieren 4-Byte-Format / LAN_X_CV_WRITE)
void XpressNetClass::CmdSchreibbefehlProgrammieren(int CV, quint8 Val)
{
  if (VelueOutOfRange(CV, 1, 1024, "Schreibbefehl Programmieren: CV")) return;
  if (VelueOutOfRange(Val, 0, 255, "Schreibbefehl Programmieren: Daten: ")) return;
  if (Z21Protocol) {
    CV = CV - 1;
    send->message({0x24, 0x12, (CV / 256), (CV % 256), Val});
    Logfile->DetailsSent("LAN_X_CV_WRITE, CV: " + str(CV + 1) + ", Value: " + str(Val));
  }
  else {
    // The version is compared as a number (see Val)
    if (XpressNetClass::Val(ZentraleSoftwareVersion) < 3.6) {
      send->message({0x23, 0x16, (CV % 256), Val});
    }
    else {
      int daten1 = 0x1C + ((CV / 256) % 4);
      send->message({0x23, daten1, (CV % 256), Val});
    }
    Logfile->DetailsSent("Schreibbefehl Programmieren 4-Byte-Format. CV: " + str(CV) + ", Value: " + str(Val));
  }
}

// V3.6: 3.2.19 / V4: 3.23 (Schreibbefehl Programmieren 3-Byte-Format - Page-Mode)
void XpressNetClass::CmdSchreibbefehlProgrammierenPageMode(int CV, int daten)
{
  if (VelueOutOfRange(CV, 1, 256, "Schreibbefehl Programmieren Pagemode: CV")) return;
  send->message({0x23, 0x17, (CV % 256), daten});
  Logfile->DetailsSent("Schreibbefehl Programmieren Pagemode. CV: " + str(CV) + ", Value: " + str(daten));
}

// V3.6: 3.2.20 / V4: 3.24 (Softwareversion der Zentrale anfordern)
void XpressNetClass::CmdSoftwareversionZentraleAnfordern()
{
  send->message({0x21, 0x21});
  Logfile->DetailsSent("Softwareversion der Zentrale anfordern");
}

// V3.6: 3.2.21 / V4: 3.26 (Status der Zentrale anfordern)
void XpressNetClass::CmdStatusZentraleAnfordern()
{
  send->message({0x21, 0x24});
  // This command may also be internally used as UDP (Z21) Keep Alive Message
  // In that case we should not log this like commands requested by the application
  if (send->waitForlanKeepAliveResponse == false) {
    Logfile->DetailsSent("Status der Zentrale anfordern");
  }
}

// V3.6: 3.2.22 / V4: 3.27 (Zentralen-Startmode setzen)
void XpressNetClass::CmdZentralenStartmodeSetzen(bool AutoStart)
{
  int value;
  if (AutoStart) value = 2;
  else value = 0;
  send->message({0x22, 0x22, value});
  Logfile->DetailsSent("Zentralen-Startmode setzen. Automatischer Start: " + tf(AutoStart));
}

// V3.6: 3.2.23 / V4: 3.37 & 3.38 / Z21: 5.1 (Schaltinformationen anfordern / LAN_X_GET_TURNOUT_INFO)
// Note: this command is intended for switches.
// The (switch) address gets divided by 4 to obtain the decoder address
void XpressNetClass::CmdSwitchInfoRequest(int address)
{
  // STEP 1: Check the address size against the ZentraleVersion and Protocol (compared as a number, see Val)
  if ((Val(ZentraleSoftwareVersion) < 3.8) && (!Z21Protocol)) {
    if (VelueOutOfRange(address, 1, 1024, "Accessory: Address")) return;
  }
  if (VelueOutOfRange(address, 1, 2048, "Accessory: Address")) return;
  // Switch numbering on the wire starts at 0. The LH100 numbering starts at 1
  int wireAddress = address - 1;
  // We need to determine the nibble for this specific switch.
  // We use temp variable to calculate the offset of the switch in this decoder
  int temp;                                               // a value between 0 and 3
  int Nibble = 0;
  temp = wireAddress % 4;                                 // the result should be 0..3
  if (temp > 1) Nibble = 1;                               // 0 and 1 become 0; 2 and 3 become 1
  // Send the command that fits for this ZentraleVersion and Protocol.
  if (Z21Protocol) {
    // Note: the XpressNet length, as contained in the four low order bits of the header,
    // seem to be incorrect! However, this is what the Z21 spec says...
    send->message({0x43, AccAH(wireAddress), AccAL(wireAddress)});
  }
  else {
    // We should set the highest bit of the byte that contains the nibble
    if (Val(ZentraleSoftwareVersion) < 3.8) {
      send->message({0x42, AccAL(wireAddress), Nibble + 128});
    }
    else {                                                // Lenz V4 spec says Command station >= 3.8 SHOULD use the new form
      send->message({0x43, AccAH(wireAddress), AccAL(wireAddress), Nibble + 128});
    }
  }
  // STEP 5: Write the command details to the logfile
  Logfile->DetailsSent("Feedback Request - Switch: " + str(address));
}

// V3.6: 3.2.23 / V4: 3.37 & 3.38 (Schaltinformationen anfordern)
// This command is intended for feedback (RS-Bus) decoders
void XpressNetClass::CmdFeedbackRequest(int address, quint8 Nibble)
{
  // STEP 1: Check the address size.
  if (VelueOutOfRange(address, 1, 128, "RS-Bus Feedback: Address")) return;
  if (Nibble != 0) Nibble = 1;                            // Ensure the nibble is 0 or 1
  // STEP 2: Call the associated ExpressNet command
  // Substract 1 from the address. The LH100 numbering starts at 1, the wire at 0
  // Also set the highest bit of the nibble
  send->message({0x42, address - 1, Nibble + 128});
  // STEP 3: Write the command details to the logfile
  Logfile->DetailsSent("Feedback Request for address: " + str(address) + "  Nibble: " + str(Nibble));
}

// V3.6: 3.2.24 / V4: 3.39 & 3.40 / Z21: 5.2 (Schaltbefehl / LAN_X_SET_TURNOUT)
void XpressNetClass::CmdAccessory(int address, quint8 position, quint8 Activate)
{
  // 2.0: comment corrected. Position 0 = diverging (red, -), 1 = straight (green, +), according to RCN-213. Some
  // command stations interpret 0 and 1 the other way around (see the README of the AP_DCC_library)
  // Was: Position 0 = straight / 1 = turn
  // Check the address size against the ZentraleVersion (compared as a number, see Val)
  if (Val(ZentraleSoftwareVersion) < 3.8) {
    if (VelueOutOfRange(address, 1, 1024, "Accessory: Address")) return;
  }
  else {
    if (VelueOutOfRange(address, 1, 2048, "Accessory: Address")) return;
  }
  address = address - 1;                                  // The LH100 numbering starts at 1, the wire at 0
  int daten = 0;                                          // Fill the Daten byte
  if (!Z21Protocol) {                                     // The Z21 uses switch addressing, instead of decoder addressing
    daten = (address % 4) * 2;                            // Store the 2 address bits that belong to this decoder in daten
  }
  daten = 128 + daten;                                    // Set the MSB
  if (Activate > 0) daten = daten + 8;                    // Set the D1 bit (Activation)
  if (position > 0) daten = daten + 1;                    // Set the D2 bit (Position)
  // Send the command that fits for this ZentraleVersion and Kennung.
  // The IF clause is used for the LZV 200 and the Z21
  // The Else clause is used for LZV100
  if ((Val(ZentraleSoftwareVersion) > 3.6) || Z21Protocol) {
    send->message({0x53, AccAH(address), AccAL(address), daten});
  }
  else {
    send->message({0x52, AccAL(address), daten});
  }
  // Write the command details to the logfile
  Logfile->DetailsSent("Accessory Command - Address: " + str(address + 1) + "  Position: " + str(position) + "  Activate: " + str(Activate));
}

// V3.6: 3.2.24 / V4: 3.39 & 3.40 (Schaltbefehl)
void XpressNetClass::CmdAccessoryAuto(int Switch, quint8 position)
{
  // Implemented in the AccAutoClass, Since it is not a standard XpressNet command.
  AccAuto->handle(Switch, position);
}

// V3.6: 3.2.25 / V4: 3.41.3 / Z21: 4.1 (Lokinformationen anfordern)
void XpressNetClass::CmdLokInfoAnfordern(int address)
{
  if (VelueOutOfRange(address, 0, 9999, "Lok Info Anfordern: Address")) return;
  if (Z21Protocol) {
    send->message({0xE3, 0xF0, AH(address), AL(address)});
  }
  else {
    send->message({0xE3, 0x00, AH(address), AL(address)});
  }
  Logfile->DetailsSent("Lok Informationen anfordern. Address: " + str(address));
}

// V3.6: 3.2.25.1 / V4: 3.41.4 (Funktionsstatus anfordern F0 - F12)
void XpressNetClass::CmdFunktionsStatusAnfordern(int address)
{
  if (VelueOutOfRange(address, 0, 9999, "Funktionsstatus Anfordern: Address")) return;
  send->message({0xE3, 0x07, AH(address), AL(address)});
  Logfile->DetailsSent("Funktionsstatus F0-F12 anfordern. Address: " + str(address));
}

// V3.6: 3.2.25.2 / V4: 3.41.5 (Funktionsstatus anfordern F13 - F28)
void XpressNetClass::CmdFunktionsStatusF13F28Anfordern(int address)
{
  if (VelueOutOfRange(address, 0, 9999, "Funktionsstatus F13-F28 Anfordern: Address")) return;
  send->message({0xE3, 0x08, AH(address), AL(address)});
  Logfile->DetailsSent("Funktionsstatus F13-F28 anfordern. Address: " + str(address));
}

// V3.6: 3.2.25.3 / V4: 3.41.7 (Funktionszustand anfordern F13 - F28)
void XpressNetClass::CmdFunktionsZustandF13F28Anfordern(int address)
{
  if (VelueOutOfRange(address, 0, 9999, "Funktionszustand F13-F28 Anfordern: Address")) return;
  send->message({0xE3, 0x09, AH(address), AL(address)});
  Logfile->DetailsSent("Funktionszustand F13-F28 anfordern. Address: " + str(address));
}

// V4: 3.41.6 (Funktionsstatus anfordern F29 - F68)
void XpressNetClass::CmdFunktionsStatusF29F68Anfordern(int address)
{
  if (VelueOutOfRange(address, 0, 9999, "Funktionsstatus F29-F68 Anfordern: Address")) return;
  send->message({0xE3, 0x0A, AH(address), AL(address)});
  Logfile->DetailsSent("Funktionsstatus F29-F68 anfordern. Address: " + str(address));
}

// V4: 3.41.8 (Funktionszustand anfordern F29 - F68)
void XpressNetClass::CmdFunktionsZustandF29F68Anfordern(int address)
{
  if (VelueOutOfRange(address, 0, 9999, "Funktionszustand F29-F68 Anfordern: Address")) return;
  send->message({0xE3, 0x0B, AH(address), AL(address)});
  Logfile->DetailsSent("Funktionszustand F29-F68 anfordern. Address: " + str(address));
}

// V3.6: 3.2.26.1 / V4: 3.42.3 / Z21: 4.2 (Lok Fahrbefehl / LAN_X_SET_LOCO_DRIVE )
void XpressNetClass::CmdFahrbefehl(int address, int Speed, int Steps, bool Forward)
{
  int RVVV = 0;                                           // The direction and speed byte
  if (VelueOutOfRange(address, 0, 9999, "Fahrbefehl: Address")) return;
  switch (Steps) {
    case 14:
      if (VelueOutOfRange(Speed, 0, 14, "Fahrbefehl: Speed")) return;
      if (Speed > 0) RVVV = Speed + 1;
      if (Forward) RVVV = RVVV + 128;
      send->message({0xE4, 0x10, AH(address), AL(address), RVVV});
      break;
    case 27:
      if (VelueOutOfRange(Speed, 0, 27, "Fahrbefehl: Speed")) return;
      RVVV = EncodeSpeed(Speed);
      if (Forward) RVVV = RVVV + 128;                     // The forward / reverse bit
      send->message({0xE4, 0x11, AH(address), AL(address), RVVV});
      break;
    case 28:
      if (VelueOutOfRange(Speed, 0, 28, "Fahrbefehl: Speed")) return;
      RVVV = EncodeSpeed(Speed);
      if (Forward) RVVV = RVVV + 128;
      send->message({0xE4, 0x12, AH(address), AL(address), RVVV});
      break;
    case 128:
      if (VelueOutOfRange(Speed, 0, 126, "Fahrbefehl: Speed")) return;
      RVVV = Speed;
      if (Speed > 0) RVVV = RVVV + 1;
      if (Forward) RVVV = RVVV + 128;
      send->message({0xE4, 0x13, AH(address), AL(address), RVVV});
      break;
    default:                                              // 2.0: not silently (1.3: nothing was sent, without an error)
      Logfile->AppendWithHost("Error Fahrbefehl: Steps must be 14, 27, 28 or 128");
      eventSendError(FehlerInDenBefehlsparametern, "Message not sent: Fahrbefehl: Steps must be 14, 27, 28 or 128 (value: " + str(Steps) + ")");
      return;
  }
  Logfile->DetailsSent("Fahrbefehl - Address: " + str(address) + ", Speed: " + str(Speed));
}

// V3.6: 3.2.26.2 / V4: 3.42.4 (Lok Funktionsbefehl)
void XpressNetClass::CmdLokFunktionBefehl(int address, quint8 group, quint8 data)
{
  // 2.0: a group outside 1..10 gives the event SendError (1.3: nothing was sent, without an error)
  if (VelueOutOfRange(group, 1, 10, "Funktions befehl: Group")) return;
  static const int kennung[11] = {0, 0x20, 0x21, 0x22, 0x23, 0x28, 0x29, 0x2A, 0x2B, 0x50, 0x51};
  send->message({0xE4, kennung[group], AH(address), AL(address), data});
  Logfile->DetailsSent("Funktions befehl. Address: " + str(address) + ", Befehl: " + str(data));
}

// V3.6: 3.2.26.4 / V4: 3.42.6 (Funktionsstatus setzen)
void XpressNetClass::CmdLokFunktionStatus(int address, quint8 group, quint8 data)
{
  // 2.0: a group outside 1..10 gives the event SendError (1.3: nothing was sent, without an error)
  if (VelueOutOfRange(group, 1, 10, "Status befehl: Group")) return;
  static const int kennung[11] = {0, 0x24, 0x25, 0x26, 0x27, 0x2C, 0x2D, 0x2E, 0x52, 0x53, 0x54};
  send->message({0xE4, kennung[group], AH(address), AL(address), data});
  Logfile->DetailsSent("Status befehl. Address: " + str(address) + ", Status: " + str(data));
}

// V3.6: 3.2.28.1 / V4: 3.43.1 (Programming on Main Byte schreiben / LAN_X_CV_POM_WRITE_BYTE)
void XpressNetClass::CmdPomWrite(int address, int CV, quint8 value)
{
  if (VelueOutOfRange(address, 0, 9999, "PoM write: Address")) return;
  if (VelueOutOfRange(CV, 1, 1024, "PoM write: CV")) return;
  CV = CV - 1;
  int cvlow = CV % 256;                                   // The low byte of the CV number
  int cvhigh = CV / 256 + 0xEC;                           // 0xEC = PoM Schreiben
  send->message({0xE6, 0x30, AH(address), AL(address), cvhigh, cvlow, value});
  Logfile->DetailsSent("Pom Write. Address: " + str(address) + ", CV: " + str(CV + 1) + ", Value: " + str(value));
}

// V3.6: 3.2.28.2 / V4: 3.43.2 (Programming on Main Byte lesen / LAN_X_CV_POM_READ_BYTE)
void XpressNetClass::CmdPomRead(int address, int CV)
{
  if (VelueOutOfRange(address, 0, 9999, "PoM Read: Address")) return;
  if (VelueOutOfRange(CV, 1, 1024, "PoM Read: CV")) return;
  CV = CV - 1;
  int cvlow = CV % 256;                                   // The low byte of the CV number
  int cvhigh = CV / 256 + 0xE4;                           // 0xE4 = PoM Lesen
  send->message({0xE6, 0x30, AH(address), AL(address), cvhigh, cvlow, 0});
  Logfile->DetailsSent("Pom Read. Address: " + str(address) + ", CV: " + str(CV + 1));
}

// V3.6: 3.2.28.3 / V4: 3.43.3 (Programming on Main Bit schreiben / LAN_X_CV_POM_WRITE_BIT)
void XpressNetClass::CmdPomBitWrite(int address, int CV, bool bitValue, quint8 bitPosition)
{
  if (VelueOutOfRange(address, 0, 9999, "PoM bit write: Address")) return;
  if (VelueOutOfRange(CV, 1, 1024, "PoM bit write: CV")) return;
  if (VelueOutOfRange(bitPosition, 0, 7, "PoM bit write: bit position")) return;
  CV = CV - 1;
  int cvlow = CV % 256;                                   // The low byte of the CV number
  // 0xE8 = PoM Bit Schreiben, as in the V3.6 spec (3.2.28.3) and the Z21 spec. The V4 spec (3.43.3) says 0x7C,
  // which seems to be an error in the V4 spec
  int cvhigh = CV / 256 + 0xE8;
  int value = bitPosition + 0xF0;                         // highest order bits must all be 1
  if (bitValue) value = value + 8;                        // bit 4 represents the new bit value
  send->message({0xE6, 0x30, AH(address), AL(address), cvhigh, cvlow, value});
  Logfile->DetailsSent("Pom Bit Write. Address: " + str(address) + ", CV: " + str(CV + 1) + ", Bit value: " + tf(bitValue) + ", Bit Position: " + str(bitPosition));
}

// V3.6 3.2.30.3 / V4: 3.44.3 (Adressanfrage Lok in Zentralenstack )
void XpressNetClass::CmdAdresAnfrageLokInStack(int address, bool Vorwarts)
{
  int Kennung;
  if (Vorwarts) Kennung = 5;
  else Kennung = 6;
  send->message({0xE3, Kennung, AH(address), AL(address)});
  Logfile->DetailsSent("Adressanfrage Lok in Zentralenstack. Address: " + str(address) + ", Vorwarts : " + tf(Vorwarts));
}

// V3.6: 3.2.31 / V4: 3.44.4 (Lok aus Stack loeschen / LAN_X_PURGE_LOCO)
void XpressNetClass::CmdLokAusStack(int address)
{
  send->message({0xE3, 0x44, AH(address), AL(address)});
  Logfile->DetailsSent("Lok aus Stack loeschen. Address: " + str(address));
}


// ==================================================================================================================
// V4 ONLY METHODS TO GENERATE COMMAND STATION SPECIFIC MESSAGES
// The commands below will be forwarded by the Interface to the Command Station

// V4: 3.25 (Erweiterte Zentralen Versionsinformation)
void XpressNetClass::CmdErweiterteZentralenVersionsinformation()
{
  send->message({0x21, 0x23});
  Logfile->DetailsSent("Erweiterte Zentralen Versionsinformation");
}

// V4: 3.30 (PoM-Ergebnis anfordern)
void XpressNetClass::CmdPoMErgebnisAnfordern()
{
  send->message({0x21, 0x27});
  Logfile->DetailsSent("PoM Programmier Ergebnis Anfordern");
}

// V4: 3.31 (Zentralen Reset auslosen)
void XpressNetClass::CmdZentralenResetAuslosen()
{
  send->message({0x21, 0x28});
  Logfile->DetailsSent("Zentralen Reset auslosen");
}

// V4: 3.33 (Modellzeit anfordern)
void XpressNetClass::CmdModellzeitAnfordern()
{
  send->message({0x21, 0x2A});
  Logfile->DetailsSent("Modellzeit anfordern");
}

// V4: 3.34 (Modellzeit stellen)
void XpressNetClass::CmdModellzeitStellen(quint8 day, quint8 hour, quint8 minute, quint8 factor)
{
  if (VelueOutOfRange(day, 0, 6, "Modellzeit: Day")) return;
  if (VelueOutOfRange(hour, 0, 23, "Modellzeit: Hour")) return;
  if (VelueOutOfRange(minute, 0, 59, "Modellzeit: Minute")) return;
  if (VelueOutOfRange(factor, 0, 31, "Modellzeit: Factor")) return;
  int daten = day * 32 + hour;
  send->message({0x24, 0x2B, daten, minute, factor});
  Logfile->DetailsSent("Modellzeit stellen");
}

// V4: 3.35 (Modellzeit anhalten - V4.0)
void XpressNetClass::CmdModellzeitAnhalten()
{
  send->message({0x21, 0x2D});
  Logfile->DetailsSent("Modellzeit anhalten");
}

// V4: 3.36 (Modellzeit starten - V4.0)
void XpressNetClass::CmdModellzeitStarten()
{
  send->message({0x21, 0x2C});
  Logfile->DetailsSent("Modellzeit starten");
}


// ==================================================================================================================
// Z21 ONLY METHODS TO GENERATE COMMAND STATION SPECIFIC MESSAGES
// The commands below will be forwarded by the Interface to the Command Station
// Note: these Z21 could not be tested, and may therefore not work as intended

// Z21: 2.16 (LAN_SET_BROADCASTFLAGS)
// 2.0: new. The flags determine which broadcasts the Z21 sends to this client, for example:
// 0x00000001: driving and switching (track power, programming mode, short circuit, stop, LAN_X_TURNOUT_INFO,
//             LAN_X_LOCO_INFO for the locomotives requested by this client)
// 0x00000002: R-Bus feedback (LAN_RMBUS_DATACHANGED)    - not decoded by the library
// 0x08000000: LocoNet detector (LAN_LOCONET_DETECTOR)   - not decoded by the library
// For the other flags, see the Z21 specification. CmdInitUdp sets the flags to Z21BroadcastFlagsDefault.
// This is not an XpressNet message: DataLen 0x08 0x00, Header 0x50 0x00, Flags (32 bits, little endian)
void XpressNetClass::CmdZ21SetBroadcastFlags(int Flags)
{
  QByteArray z21Message(8, 0);
  z21Message[0] = char(0x08);
  z21Message[1] = char(0x00);
  z21Message[2] = char(0x50);
  z21Message[3] = char(0x00);
  z21Message[4] = char(Flags & 0xFF);
  z21Message[5] = char((Flags >> 8) & 0xFF);
  z21Message[6] = char((Flags >> 16) & 0xFF);
  z21Message[7] = char((Flags >> 24) & 0xFF);
  send->transmitZ21(z21Message);
  Logfile->DetailsSent("Z21: CmdZ21SetBroadcastFlags. Flags: 0x" + QString("%1").arg(quint32(Flags), 8, 16, QChar('0')).toUpper());
}

// Z21: 2.15 (LAN_X_GET_FIRMWARE_VERSION)
void XpressNetClass::CmdZ21GetFirmwareVersion()
{
  send->message({0xF1, 0x0A});
  Logfile->DetailsSent("Z21: CmdZ21GetFirmwareVersion");
}

// Z21: 4.3.1 (LAN_X_SET_LOCO_FUNCTION)
void XpressNetClass::CmdZ21SetLocoFunction(int address, quint8 CV, quint8 TT)
{
  // TT Umschalttyp: 00=aus, 01=ein, 10=umschalten,11=nicht erlaubt
  // CV: 0x00=F0 (Licht), 0x01=F1 usw. bis F31
  if (VelueOutOfRange(CV, 0, 31, "LAN_X_SET_LOCO_FUNCTION: CV")) return;
  if (VelueOutOfRange(TT, 0, 2, "LAN_X_SET_LOCO_FUNCTION: TT")) return;
  int TTNNNNNN = TT * 64 + CV;
  send->message({0xE4, 0xF8, AH(address), AL(address), TTNNNNNN});
  Logfile->DetailsSent("Z21: CmdZ21SetLocoFunction. CV: " + str(CV) + ", TT: " + str(TT));
}

// Z21: 5.4 LAN_X_SET_EXT_ACCESSORY
void XpressNetClass::CmdZ21SetExtAccessory(int address, quint8 value)
{
  if (VelueOutOfRange(address, 1, 2048, "Extended Accessory: Address")) return;
  // The first "user address" is 1, which is represented as 4 in the Xpressnet message
  int rawAddress = address + 3;
  if (rawAddress >= 2048) rawAddress = rawAddress - 2048;
  send->message({0x54, (rawAddress / 256), (rawAddress % 256), value, 0x00});
  Logfile->DetailsSent("Z21: CmdZ21SetExtAccessory. Address: " + str(address) + ", Value: " + str(value));
}

// Z21: 5.5 LAN_X_GET_EXT_ACCESSORY_INFO
void XpressNetClass::CmdZ21GetExtAccessory(int address)
{
  if (VelueOutOfRange(address, 1, 2048, "Extended Accessory: Address")) return;
  // The first "user address" is 1, which is represented as 4 in the Xpressnet message
  int rawAddress = address + 3;
  if (rawAddress >= 2048) rawAddress = rawAddress - 2048;
  // As opposed to normal accesories, no address correction takes place
  // As in the Z21 spec (5.5, DataLen 0x09): X-Header 0x44, Adr_MSB, Adr_LSB, 0x00
  send->message({0x44, (rawAddress / 256), (rawAddress % 256), 0x00});
  Logfile->DetailsSent("Z21: CmdZ21SetExtAccessory. Address: " + str(address));
}

// Z21: 6.9 (LAN_X_CV_POM_ACCESSORY_WRITE_BYTE)
void XpressNetClass::CmdZ21AccPomWrite(int decoderAddress, quint8 CDDD, int CV, quint8 value)
{
  if (VelueOutOfRange(decoderAddress, 0, 511, "Accessory PoM write: Decoder Address")) return;
  if (VelueOutOfRange(CDDD, 0, 15, "Accessory PoM write: CDDD")) return;
  if (VelueOutOfRange(CV, 1, 1024, "Accessory PoM write: CV")) return;
  CV = CV - 1;
  // Es gilt: aaaaaAAAACDDD = (Decoder Addresse & 0x1FF) << 4) | CDDD;
  // Falls CDDD=0000, dann bezieht sich die CV auf den ganzen Decoder.
  // Falls C=1, so ist DD die Nummer des zu programmierenden Ausgangs.
  int aaaaa = decoderAddress / 16;
  int AAAACDDD = (decoderAddress % 16) * 16 + CDDD;
  // CV bytes
  int cvlow = CV % 256;                                   // The low byte of the CV number
  int cvhigh = CV / 256 + 0xEC;                           // 0xEC = Acc PoM Schreiben
  send->message({0xE6, 0x31, aaaaa, AAAACDDD, cvhigh, cvlow, value});
  Logfile->DetailsSent("Accessory PoM Write. Decoder address: " + str(decoderAddress) + ", CDDD: " + str(CDDD) + ", CV: " + str(CV + 1) + ", Value: " + str(value));
}

// Z21: 6.10 (LAN_X_CV_POM_ ACCESSORY_WRITE_BIT )
void XpressNetClass::CmdZ21AccPomBitWrite(int decoderAddress, quint8 CDDD, int CV, bool bitValue, quint8 bitPosition)
{
  if (VelueOutOfRange(decoderAddress, 0, 511, "Accessory PoM Bit write: Decoder Address")) return;
  if (VelueOutOfRange(CDDD, 0, 15, "Accessory PoM Bit write: CDDD")) return;
  if (VelueOutOfRange(CV, 1, 1024, "PoM Bit write: CV")) return;
  CV = CV - 1;
  // Es gilt: aaaaaAAAACDDD = (Decoder Addresse & 0x1FF) << 4) | CDDD;
  // Falls CDDD=0000, dann bezieht sich die CV auf den ganzen Decoder.
  // Falls C=1, so ist DD die Nummer des zu programmierenden Ausgangs.
  int aaaaa = decoderAddress / 16;
  int AAAACDDD = (decoderAddress % 16) * 16 + CDDD;
  // CV bytes
  int cvlow = CV % 256;                                   // The low byte of the CV number
  int cvhigh = CV / 256 + 0xE8;                           // 0xE8 = Acc PoM Bit Schreiben
  // DB5: 0000VPPP
  int VPPP = bitPosition;
  if (bitValue) VPPP = VPPP + 8;                          // bit 4 represents the new bit value
  send->message({0xE6, 0x31, aaaaa, AAAACDDD, cvhigh, cvlow, VPPP});
  Logfile->DetailsSent("Accessory PoM Bit Write. Decoder address: " + str(decoderAddress) + ", CDDD: " + str(CDDD) + ", CV: " + str(CV + 1) + ", bitValue: " + tf(bitValue) + ", bitPosition: " + str(bitPosition));
}

// Z21: 6.11 (LAN_X_CV_POM_ACCESSORY_READ_BYTE)
void XpressNetClass::CmdZ21AccPomRead(int decoderAddress, quint8 CDDD, int CV)
{
  if (VelueOutOfRange(decoderAddress, 0, 511, "Accessory PoM read: Decoder Address")) return;
  if (VelueOutOfRange(CDDD, 0, 15, "Accessory PoM read: CDDD")) return;
  if (VelueOutOfRange(CV, 1, 1024, "Accessory PoM read: CV")) return;
  CV = CV - 1;
  // Es gilt: aaaaaAAAACDDD = (Decoder Addresse & 0x1FF) << 4) | CDDD;
  // Falls CDDD=0000, dann bezieht sich die CV auf den ganzen Decoder.
  // Falls C=1, so ist DD die Nummer des zu programmierenden Ausgangs.
  int aaaaa = decoderAddress / 16;
  int AAAACDDD = (decoderAddress % 16) * 16 + CDDD;
  // CV bytes
  int cvlow = CV % 256;                                   // The low byte of the CV number
  int cvhigh = CV / 256 + 0xE4;                           // 0xE4 = Acc PoM Lesen
  send->message({0xE6, 0x31, aaaaa, AAAACDDD, cvhigh, cvlow, 0x00});
  Logfile->DetailsSent("Accessory PoM Read. Decoder address: " + str(decoderAddress) + ", CDDD: " + str(CDDD) + ", CV: " + str(CV + 1));
}

// Z21: 6.12 (LAN_X_MM_WRITE_BYTE )
void XpressNetClass::CmdZ21MmWriteByte(quint8 RegAdr, quint8 value)
{
  send->message({0x24, 0xFF, 0x00, RegAdr, value});
  Logfile->DetailsSent("Z21: CmdZ21MmWriteByte");
}


// ==================================================================================================================
// OpenDCC ONLY METHODS TO GENERATE COMMAND STATION SPECIFIC MESSAGES
// The commands below will be forwarded by the Interface to the Command Station
// Note: these commands could not be tested, and may therefore not work as intended
void XpressNetClass::CmdOpenDCCSetExtAccessory(int address, quint8 value)
{
  if (VelueOutOfRange(address, 1, 2048, "Ext. Accessory: Address")) return;
  if (VelueOutOfRange(value, 0, 31, "Ext. Accessory: Value")) return;
  int byte1 = value * 8;                                  // Shift value 3 bits left
  byte1 = byte1 + address / 256;                          // 3 highest address bits
  int byte2 = address % 256;                              // 8 lowest address bits
  send->message({0x13, 0x01, byte1, byte2});
  Logfile->DetailsSent("OPENDCC: Extended Accessory Command. Address = " + str(address) + ", Value = " + str(value));
}


// ******************************************************************************************************************
// ***************************************************** EVENTS *****************************************************
// ******************************************************************************************************************
// RAISING EVENTS: TCP, USB AND XPRESSNET FOR ERRORS, STATUS CHANGES AND FEEDBACK INFO
// The methods below forward events from lower level objects to the library user
// In addition Logfile messages are written, if needed.
void XpressNetClass::eventInterfaceConnected()
{
  emit InterfaceConnected();
  Logfile->Details("Interface connection established");
  // In case of a serial connection, it seems we must wait 100ms before the interface is stable
  if (connectionType == serialPort) QThread::msleep(100);
  // Retrieve version and status info regarding the Command Station
  requestZentraleInfo();
}

void XpressNetClass::requestZentraleInfo()
{
  CmdSoftwareversionZentraleAnfordern();
  CmdStatusZentraleAnfordern();
}

// 2.0: logText: the complete description for the logfile (default: Description)
void XpressNetClass::eventTcpError(const QString &Description, const QString &logText)
{
  emit TcpError(Description);
  Logfile->Errors("TCP Error: " + (logText.isEmpty() ? Description : logText));
}

void XpressNetClass::eventTcpClosedByRemoteHost()
{
  // The connection is cleaned up before the event is raised (VB6: after). The event handler of the application
  // may then open a new connection (for example CmdConnectTcp), which should not be closed afterwards
  if (tcp != nullptr) {
    tcp->closeConnection();
    tcp->deleteLater();                                   // Not delete: we may be called from a method of tcp
  }
  tcp = nullptr;
  connectionType = none;
  emit TcpClosedByRemoteHost();
  Logfile->Errors("TCP Connection closed by remote host");
}


// ------------------------------------------------------------------------------------------------------------------
// Interface specific events
// V3.6: 1.6 (Versionsnummer des Interface 23151 feststellen)
void XpressNetClass::eventInterfaceVersion(quint8 daten1, quint8 daten2)
{
  QString Version = str(daten1 / 16) + "." + str(daten1 % 16);
  QString code = str(daten2 / 16) + "." + str(daten2 % 16);
  emit InterfaceVersion(Version, code);
  Logfile->DetailsReceived("Interface - Version: " + Version + "  Code: " + code);
}

// V3.6: 1.7 (XpressNet Address) / V3.6: 2.1 (Interface Status Befehl)
void XpressNetClass::eventXpressNetAddress(quint8 address)
{
  // This event is triggered as response to two commands:
  // XpressNet Address (CmdXpressNetAddress - V3.6: 1.7) and
  // Interface Status Befehl (CmdInterfaceStatus - V3.6: 2.1)
  // CmdInterfaceStatus will be called internally by the library, whenever the
  // keep alive timer expires (send->lanKeepAliveTimer). In such case,
  // the response to the keep alive message should not be returned (as event)
  // to the application program, but instead be kept within the library.
  // To distinguish between both cases, the keep alive timer sets the
  // waitForlanKeepAliveResponse flag
  if (send->waitForlanKeepAliveResponse) {
    send->waitForlanKeepAliveResponse = false;
    Logfile->DetailsReceived("TCP Keep Alive Response");
  }
  else {
    emit XpressNetAddress(address);
    Logfile->DetailsReceived("XpressNet - Address: " + str(address));
  }
}

// V3.6: 2.2 (XpressNet Version)
void XpressNetClass::eventXpressNetVersion(quint8 daten)
{
  QString Version = str(daten / 16) + "." + str(daten % 16);
  emit XpressNetVersion(Version);
  Logfile->DetailsReceived("XpressNet - Version: " + Version);
}

// V3.6: 2.3 (Freie Verbindungen)
void XpressNetClass::eventInterfaceFreieVerbindungen(quint8 number)
{
  emit InterfaceFreieVerbindungen(number);
  Logfile->DetailsReceived("Interface - Free TCP Sockets: " + str(number));
}

// ------------------------------------------------------------------------------------------------------------------
// Xpressnet specific events
// V3.6: 3.1.1.1, V4: 2.4.1, Z21: 2.8 (LAN_X_BC_TRACK_POWER_ON)
void XpressNetClass::eventAllesAn()
{
  ZentraleNotAus = false;
  ZentraleNotHalt = false;
  ZentraleProgrammierMode = false;
  emit AllesAn();
  Logfile->DetailsReceived("Broadcast: Alles An");
}

// V3.6: 3.1.1.2, V4: 2.4.2, Z21: 2.7 (LAN_X_BC_TRACK_POWER_OFF)
void XpressNetClass::eventNotAus()
{
  ZentraleNotAus = true;
  emit NotAus();
  Logfile->DetailsReceived("Broadcast: Alles Aus (Notaus)");
}

// V3.6: 3.1.1.3, V4: 2.4.3, Z21: 2.14 (LAN_X_BC_STOPPED)
void XpressNetClass::eventNotHalt()
{
  ZentraleNotHalt = true;
  emit NotHalt();
  Logfile->DetailsReceived("Broadcast: Alle Loks Aus - Nothalt");
}

// V3.6: 3.1.1.4, V4: 2.4.4, Z21: 2.9 (LAN_X_BC_PROGRAMMING_MODE)
void XpressNetClass::eventProgrammierMode()
{
  ZentraleProgrammierMode = true;
  emit ProgrammierMode();
  Logfile->DetailsReceived("Broadcast: Programmiermode");
}

// To simplify the application program, we generate different feedback events for switches and for
// feedback (occupancy) decoders. The difference between both can be determined by the TT bits
// V3.6: 3.1.1.5, V4: 2.4.5, V4: 2.4.6 (BC RuckMeldung) / V3.6: 3.1.8, V4: 2.15, V4: 2.16 (Schaltinformation)
void XpressNetClass::eventFeedback(int address, quint8 Nibble, quint8 ttBits, quint8 iBit, quint8 data)
{
  if (ttBits == 2) {                                      // normal feedback decoder
    // We add 1 to the RS-Bus address, since addresses on the wire range from 0..127. We display these as 1..128
    address = address + 1;
    emit Feedback(address, Nibble, data);
    Logfile->DetailsReceived("Feedback Message received for RS-Bus address: " + str(address) + "  Data: " + str(data) + "  Nibble: " + str(Nibble));
  }
  else {                                                  // switch decoder
    // Adress is the decoder address, but we will return a switch address pair
    int switchAddress;
    if (!Z21Protocol) {
      switchAddress = (address * 4) + (Nibble * 2) + 1;
    }
    else {
      switchAddress = address + 1;
    }
    emit SwitchFeedback(switchAddress, ttBits, iBit, data);
    Logfile->DetailsReceived("Feedback Message received for switch addresses: " + str(switchAddress) + "/" + str(switchAddress + 1) + "  Data: " + str(data) + "  TT-Bits: " + str(ttBits) + " I-Bit: " + str(iBit));
  }
}

// V3.6: 3.1.2.1, V4: 2.5.1, Z21: 2.10 (Programmierinfo Kurzschluss / LAN_X_BC_TRACK_SHORT_CIRCUIT)
void XpressNetClass::eventProgrammierinfoKurzschluss()
{
  emit ProgrammierinfoKurzschluss();
  Logfile->DetailsReceived("Programmierinfo Kurzschluss");
}

// V3.6: 3.1.2.2, V4: 2.5.2, Z21: 6.4 (Programmierinfo Daten nicht gefunden / LAN_X_CV_NACK)
void XpressNetClass::eventProgrammierinfoDatenNichtGefunden()
{
  emit ProgrammierinfoDatenNichtGefunden();
  Logfile->DetailsReceived("Programmierinfo Daten nicht gefunden");
}

// V3.6: 3.1.2.3, V4: 2.5.3 (Programmierinfo Zentrale Busy)
void XpressNetClass::eventProgrammierinfoZentraleBusy()
{
  emit ProgrammierinfoZentraleBusy();
  Logfile->DetailsReceived("Programmierinfo Zentrale Busy");
}

// V3.6: 3.1.2.4, V4: 2.5.4 (Programmierinfo Zentrale Bereit)
void XpressNetClass::eventProgrammierinfoZentraleBereit()
{
  emit ProgrammierinfoZentraleBereit();
  Logfile->DetailsReceived("Programmierinfo Zentrale Bereit");
}

// V3.6 3.1.2.5 / V4: 2.5.5 (Programmierinfo Daten 3-Byte-Format)
void XpressNetClass::eventProgInfoPageRegister(quint8 EEPromAddress, quint8 value)
{
  emit ProgrammierinfoPageRegister(EEPromAddress, value);
  Logfile->DetailsReceived("Programmierinfo Page / Register. EEPromAddress: " + str(EEPromAddress) + ", Value: " + str(value));
}

// V3.6: 3.1.2.6 - 3.1.2.9, V4: 2.5.6 - 2.5.9 (Programmierinfo CV1..1024)
void XpressNetClass::eventProgInfo(quint8 CvRange, quint8 daten1, quint8 value)
{
  int CV = daten1;
  switch (CvRange) {
    case 1:
      if (CV == 0) {
        // The version is compared as a number (see Val)
        if (Val(ZentraleSoftwareVersion) < 3.6) CV = 256;
        else CV = 1024;
      }
      break;
    case 2: CV = CV + 256; break;
    case 3: CV = CV + 512; break;
    case 4: CV = CV + 768; break;
  }
  emit ProgrammierinfoCV(CV, value);
  Logfile->DetailsReceived("Programmierinfo. CV: " + str(CV) + ", Value: " + str(value));
}

// Z21: 6.5 (LAN_X_CV_RESULT)
void XpressNetClass::eventProgInfoZ21(quint8 CVAdr_MSB, quint8 CVAdr_LSB, quint8 value)
{
  int CV = (CVAdr_MSB * 256) + CVAdr_LSB + 1;
  emit ProgrammierinfoCV(CV, value);
  Logfile->DetailsReceived("Programmierinfo Z21. CV: " + str(CV) + ", Value: " + str(value));
}

// V4: 2.6 (Service Variable melden)
void XpressNetClass::eventServiceVariableMelden(quint8 SvAddress, quint8 SvValue)
{
  emit ServiceVariable(SvAddress, SvValue);
  Logfile->DetailsReceived("Service Variable melden. SV Address: " + str(SvAddress) + ", SV Value: " + str(SvValue));
}

// V3.6: 3.1.3, V4: 2.7.2, Z21: 2.3 (Softwareversion Zentrale / LAN_X_GET_VERSION)
void XpressNetClass::eventSoftwareVersionZentrale(quint8 daten2, quint8 daten3)
{
  QString Version = str(daten2 / 16) + "." + str(daten2 % 16);
  ZentraleSoftwareVersion = Version;
  ZentraleKennung = daten3;
  emit ZentraleVersion(Version);
  // 2.0: with the name of the command station
  Logfile->DetailsReceived("Version Zentrale: " + Version + ", Kennung: " + str(ZentraleKennung) + " (" + zentraleName(ZentraleKennung) + ")");
}

// 2.0: The name of the command station, for the logfile (Zentralen-Kennung, V3.6: 3.1.3)
QString XpressNetClass::zentraleName(quint8 kennung)
{
  switch (kennung) {
    case LZ100: return "LZ 100";
    case LH200: return "LH 200";
    case DPC: return "DPC (Compact / Commander)";
    case CntrPlus: return "Control Plus";
    case LokMausII: return "LokMaus II";
    case LDH: return "LDH";
    case multiMAUS: return "multiMAUS";
    case Z21: return "Z21 / Yamorc";
    case OpenDcc: return "OpenDCC";
    case OpenDccXp: return "OpenDccXp";
    default: return "unknown";
  }
}

// V3.6: 3.1.4, V4: 2.8, Z21: 2.12 (Status Zentrale / LAN_X_STATUS_CHANGED)
void XpressNetClass::eventStatusZentrale(quint8 data)
{
  // With Z21, we internally use the StatusZentrale command as "Keep Alive" Message
  // If we use the StatusZentrale command in this way, we need not inform the application
  // about the result. In that case it is sufficient to tell the remaining program that
  // we no longer wait for the Keep Alive Response. Just log it that we received
  // the response, and exit
  if ((connectionType == lanUdpPort) && send->waitForlanKeepAliveResponse) {
    send->waitForlanKeepAliveResponse = false;
    Logfile->DetailsReceived("UDP Keep Alive Response");
    return;
  }
  // If we are here, this is not a Z21 Keep Alive, but a response to a normal request.
  ZentraleNotHalt = bitValue(data, 0);
  ZentraleNotAus = bitValue(data, 1);
  if (!Z21Protocol) ZentraleStartmodeAuto = bitValue(data, 2);
  if (!Z21Protocol) ZentraleProgrammierMode = bitValue(data, 3);
  if (Z21Protocol) ZentraleKurzSchluss = bitValue(data, 2);
  if (Z21Protocol) ZentraleProgrammierMode = bitValue(data, 5);
  ZentraleKaltstart = bitValue(data, 6);
  ZentraleRamCheckFehler = bitValue(data, 7);
  if (ZentraleNotHalt) {
    emit NotHalt();
    Logfile->DetailsReceived("Startup: NotHalt");
  }
  if (ZentraleNotAus) {
    emit NotAus();
    Logfile->DetailsReceived("Startup: NotAus");
  }
  if (ZentraleProgrammierMode) {
    emit ProgrammierMode();
    Logfile->DetailsReceived("Startup: ProgrammierMode");
  }
  if (ZentraleRamCheckFehler) {
    Logfile->Errors("Startup: Zentrale - Ram Check Fehler");
  }
  Logfile->DetailsReceived("Status Zentrale: " + str(data));
}

// V4: 2.10 (PoM Ergebnis melden)
void XpressNetClass::eventPomErgebnisMelden(quint8 AdrH, quint8 AdrL, quint8 CvValue)
{
  int LokAddress = AdrH * 256 + AdrL;
  emit PomErgebnis(LokAddress, CvValue);
  Logfile->DetailsReceived("PomErgebnisMelden. Lok Address:: " + str(LokAddress) + ", CV Value:" + str(CvValue));
}

// V3.6: 3.1.9.1, V4: 2.19.1 (Lok Info Normale Lok)
void XpressNetClass::eventNormaleLokinfo(quint8 Kennung, quint8 Speed, quint8 F0_F4, quint8 F5_F12)
{
  bool Besetzt = bitValue(Kennung, 3);
  int Fahrstufen = 0;
  switch (Kennung % 8) {                                  // Analyse bits 0..2
    case 0: Fahrstufen = 14; break;
    case 1: Fahrstufen = 27; break;
    case 2: Fahrstufen = 28; break;
    case 4: Fahrstufen = 128; break;
  }
  bool Forwarts = bitValue(Speed, 7);
  // The speed byte is decoded according to the speed steps
  int Geschwindigkeit = DecodeSpeedSteps(Speed, Fahrstufen);
  quint8 F5_F8 = quint8(F5_F12 % 16);                     // Gruppe 2 - take the lowest 4 bits
  quint8 F9_F12 = quint8(F5_F12 / 16);                    // Gruppe 3 - take the highest 4 bits
  emit NormaleLokinfo(Besetzt, Fahrstufen, Geschwindigkeit, Forwarts, F0_F4, F5_F8, F9_F12);
  Logfile->DetailsReceived("Lokinformation (normal) - Besetzt: " + tf(Besetzt) + ", Speed: " + str(Geschwindigkeit) + ", Stufen: " + str(Fahrstufen) + ", Forwarts: " + tf(Forwarts) + ", F0..F4 " + str(F0_F4) + ", F5..F8 " + str(F5_F8) + ", F9..F12 " + str(F9_F12));
}

// Z21: 4.4 (LAN_X_LOCO_INFO)
void XpressNetClass::eventZ21Lokinfo(quint8 Adr_MSB, quint8 Adr_LSB, quint8 Kennung, quint8 Speed, quint8 F0_F4, quint8 F5_F12, quint8 F13_F20, quint8 F21_F28)
{
  int LokAddress = (Adr_MSB & 0x3F) * 256 + Adr_LSB;
  bool Besetzt = bitValue(Kennung, 3);
  int Fahrstufen = 0;
  switch (Kennung % 8) {                                  // Analyse bits 0..2
    case 0: Fahrstufen = 14; break;
    case 1: Fahrstufen = 27; break;
    case 2: Fahrstufen = 28; break;
    case 4: Fahrstufen = 128; break;
  }
  bool Forwarts = bitValue(Speed, 7);
  // The speed byte is decoded according to the speed steps
  int Geschwindigkeit = DecodeSpeedSteps(Speed, Fahrstufen);
  quint8 F5_F8 = quint8(F5_F12 % 16);                     // Gruppe 2 - take the lowest 4 bits
  quint8 F9_F12 = quint8(F5_F12 / 16);                    // Gruppe 3 - take the highest 4 bits
  emit Z21Lokinfo(LokAddress, Besetzt, Fahrstufen, Geschwindigkeit, Forwarts, F0_F4, F5_F8, F9_F12, F13_F20, F21_F28);
  Logfile->DetailsReceived("Lokinformation (Z21) - Address: " + str(LokAddress) + ", Besetzt: " + tf(Besetzt) + ", Speed: " + str(Geschwindigkeit) + ", Stufen: " + str(Fahrstufen) + ", Forwarts: " + tf(Forwarts) + ", F0..F4 " + str(F0_F4) + ", F5..F8 " + str(F5_F8) + ", F9..F12 " + str(F9_F12) + ", F13..F20 " + str(F13_F20) + ", F21..F28 " + str(F21_F28));
}

// V3.6: 3.1.9.2, V4: 2.19.2 (Funktionszustand F13 bis F28)
void XpressNetClass::eventFunctionsZustandF13F20(quint8 F13_F20, quint8 F21_F28)
{
  emit FunctionsZustandF13F20(F13_F20, F21_F28);
  Logfile->DetailsReceived("Funktionszustand - F13 bis F20: " + str(F13_F20) + ", F21 bis F28: " + str(F21_F28));
}

// V4: 2.19.3 (Funktionszustand F29 bis F68)
void XpressNetClass::eventFunctionsZustandF29F68(quint8 F29_F36, quint8 F37_F44, quint8 F45_F52, quint8 F53_F60, quint8 F61_F68)
{
  emit FunctionsZustandF29F68(F29_F36, F37_F44, F45_F52, F53_F60, F61_F68);
  Logfile->DetailsReceived("Funktionszustand - F29 bis F68: " + str(F29_F36) + ", " + str(F37_F44) + ", " + str(F45_F52) + ", " + str(F53_F60) + ", " + str(F61_F68));
}

// V3.6: 3.1.11, V4: 2.19.8 (Funktionsstatus F0 bis F12 der angefragten Lok)
void XpressNetClass::eventFunctionStatusF0F12(quint8 F0_F4, quint8 F5_F12)
{
  quint8 F5_F8 = quint8(F5_F12 % 16);                     // Gruppe 2 - take the lowest 4 bits
  quint8 F9_F12 = quint8(F5_F12 / 16);                    // Gruppe 3 - take the highest 4 bits
  emit FunctionStatusF0F12(F0_F4, F5_F8, F9_F12);
  Logfile->DetailsReceived("Funktionsstatus - F0..F4 " + str(F0_F4) + ", F5..F8 " + str(F5_F8) + ", F9..F12 " + str(F9_F12));
}

// V3.6: 3.1.12, V4: 2.20 (Funktionsstatus F13 bis F28)
void XpressNetClass::eventFunctionStatusF13F20(quint8 F13_F20, quint8 F21_F28, quint8 RefreshMode)
{
  emit FunctionStatusF13F20(F13_F20, F21_F28, RefreshMode);
  Logfile->DetailsReceived("Funktionsstatus - F13 bis F20: " + str(F13_F20) + ", F21 bis F28: " + str(F21_F28) + ", Refresh Mode: " + str(RefreshMode));
}

// V4: 2.21 (Funktionsstatus F29 bis F68)
void XpressNetClass::eventFunctionStatusF29F68(quint8 F29_F36, quint8 F37_F44, quint8 F45_F52, quint8 F53_F60, quint8 F61_F68)
{
  emit FunctionStatusF29F68(F29_F36, F37_F44, F45_F52, F53_F60, F61_F68);
  Logfile->DetailsReceived("Funktionsstatus - F29 bis F68: " + str(F29_F36) + ", " + str(F37_F44) + ", " + str(F45_F52) + ", " + str(F53_F60) + ", " + str(F61_F68));
}

// V3.6: 3.1.14, V4: 2.24 (Fehlermeldung)
void XpressNetClass::eventFehlermeldung(quint8 data)
{
  if (bitValue(data, 7)) {                                // Highest order bit must have been set
    quint8 value = quint8(data % 16);                     // Take the four lowest order bits
    emit Fehlermeldung(value);
    if (LogWarnings) {
      Logfile->AppendWithHost("<= Fehlermeldung: " + str(value));
    }
  }
}

// V4: 2.4.7 / V4: 2.11 (BC: Modellzeit / Modellzeit melden)
void XpressNetClass::eventModellZeit(quint8 daten1, quint8 daten2, quint8 factor)
{
  quint8 day = quint8(daten1 / 32);
  quint8 hour = quint8(daten1 % 32);
  quint8 minute = quint8(daten2 % 64);
  // run = daten2 \ 128                       ' Seems Run is always zero.... Error in LZV200??
  emit Modellzeit(day, hour, minute, factor);
  // We do not log Modellzeit messages in the logfile
}

// V4: 2.9 (Erweiterte Versionsinformation melden)
void XpressNetClass::eventErweiterteVersionZentrale(quint8 ZBldH, quint8 ZBldL, quint8 RMVer, quint8 RMBldH, quint8 RMBldL, quint8 BIvers)
{
  QString BuildZentrale = str(ZBldH * 256 + ZBldL);
  QString VersionRuckMelder = str(RMVer / 16) + "." + str(RMVer % 16);
  QString BuildRuckMelder = str(RMBldH * 256 + RMBldL);
  QString VersionBootloader = str(BIvers / 16) + "." + str(BIvers % 16);
  emit ErweiterteZentraleVersion(BuildZentrale, VersionRuckMelder, BuildRuckMelder, VersionBootloader);
  Logfile->DetailsReceived("Erweiterte Versionsinformation - BuildZentrale = " + BuildZentrale + ", VersionRuckMelder = " + VersionRuckMelder + ", BuildRuckMelder = " + BuildRuckMelder + ", VersionBootloader = " + VersionBootloader);
}

// Z21: 2.15 (Antwort auf LAN_X_GET_FIRMWARE_VERSION)
void XpressNetClass::eventz21FirmwareVersion(quint8 V_MSB, quint8 V_LSB)
{
  // The version is BCD coded (Z21 spec 2.15: 0x01 0x23 = V1.23)
  QString Version = QString::number(V_MSB, 16).toUpper() + "." + QString("%1").arg(V_LSB, 2, 16, QChar('0')).toUpper();
  emit z21FirmwareVersion(Version);
  Logfile->DetailsReceived("Z21 Firmware - Version = " + Version);
}

// Z21: 5.6 (ExtendedAccessoryInfo(ByVal address As Integer, ByVal Value As Byte, ByVal Valid As Boolean))
void XpressNetClass::eventExtendedAccessoryInfo(quint8 Adr_MSB, quint8 Adr_LSB, quint8 DDDDDDDD, quint8 Status)
{
  int rawAddress = (Adr_MSB * 256) + Adr_LSB;
  if (rawAddress <= 3) rawAddress = rawAddress + 2048;
  int address = rawAddress - 3;                           // The first (XpressNet) address is 4, and presented to the user as 1
  emit ExtendedAccessoryInfo(address, DDDDDDDD, Status);
  Logfile->DetailsReceived("Extended Accessory Info - Address: " + str(address) + ", Value: " + str(DDDDDDDD) + ", Status: " + str(Status));
}

// ------------------------------------------------------------------------------------------------------------------
// 2.0: Sending events (see XpressNetSend.cpp). The logfile is written by the caller
void XpressNetClass::eventSendError(SendErrorReasonEnum reason, const QString &description)
{
  emit SendError(quint8(reason), description);
}

void XpressNetClass::eventInterfaceNichtAdressiert()
{
  emit InterfaceNichtAdressiert();
}

void XpressNetClass::eventInterfaceWiederAdressiert()
{
  emit InterfaceWiederAdressiert();
}


// ******************************************************************************************************************
// ************************************************* INTERNAL USE ***************************************************
// ******************************************************************************************************************
// Do we have a connection on which we can send?
bool XpressNetClass::isConnected()
{
  switch (connectionType) {
    case lanTcpPort: return (tcp != nullptr) && tcp->connected;
    case lanUdpPort: return (udp != nullptr) && udp->ready;
    case usbPort: return (usb != nullptr) && usb->isOpen;
    case serialPort: return (usb != nullptr) && usb->isOpen;
    default: return false;
  }
}

// Close all connections
void XpressNetClass::closeConnections()
{
  if (tcp != nullptr) tcp->closeConnection();
  // 2.0: log off from the Z21 (Z21 spec 2.2), via the same socket (port) as used for the login
  if ((udp != nullptr) && (connectionType == lanUdpPort)) {
    send->transmitZ21(QByteArray("\x04\x00\x30\x00", 4));
    Logfile->DetailsSent("Z21: LAN_LOGOFF");
  }
  if (udp != nullptr) udp->close();
  if (usb != nullptr) usb->closePort();
  // The objects are deleted later: this method may be called from a method of one of these objects
  if (tcp != nullptr) tcp->deleteLater();
  if (udp != nullptr) udp->deleteLater();
  if (usb != nullptr) usb->deleteLater();
  tcp = nullptr;
  udp = nullptr;
  usb = nullptr;
  connectionType = none;
}

// Some bit manipulation functions
bool XpressNetClass::bitValue(int data, int index)
{
  return ((data >> index) & 1) == 1;
}

// support function to check is Value is between some predefined bounds
// 2.0: a value out of range gives the event SendError (the message is not sent)
bool XpressNetClass::VelueOutOfRange(int value, int min, int max, const QString &LogText)
{
  bool outOfRange = false;
  if (value < min) {
    Logfile->AppendWithHost("Error " + LogText + " must be >= " + str(min));
    eventSendError(FehlerInDenBefehlsparametern, "Message not sent: " + LogText + " must be >= " + str(min) + " (value: " + str(value) + ")");
    outOfRange = true;
  }
  if (value > max) {
    Logfile->AppendWithHost("Error " + LogText + " must be <= " + str(max));
    eventSendError(FehlerInDenBefehlsparametern, "Message not sent: " + LogText + " must be <= " + str(max) + " (value: " + str(value) + ")");
    outOfRange = true;
  }
  return outOfRange;
}

// Functions for Accessory addresses
int XpressNetClass::AccAL(int address)
{
  // Calculation of the low byte of a accessory address
  if (Z21Protocol) {
    return address % 256;                                 // Select the last 8-bits
  }
  else {
    if (address >= 1024) {                                // We already assured that the command station >= V3.8
      return (address - 1024) / 4;                        // Integer division. Result fits in an 8-bit byte
    }
    else {
      return address / 4;                                 // Integer division
    }
  }
}

int XpressNetClass::AccAH(int address)
{
  // Calculation of the high byte of a accessory address
  if (Z21Protocol) {
    return address / 256;                                 // Remove the last 8-bits (shift right)
  }
  else {                                                  // Lenz uses decoder addressing
    return address / 1024;                                // Remove the last 10-bits (shift right)
  }
}

// Functions for lok addresses and speed
int XpressNetClass::AL(int address)
{
  // Calculation of the low byte of a loco address
  return address % 256;
}

int XpressNetClass::AH(int address)
{
  // Calculation of the high byte of a loco address
  if (address < 100) return 0;
  else return address / 256 + 0xC0;
}

int XpressNetClass::EncodeSpeed(int Speed)
{
  // Calculates the speed byte for decoders with 27/28 speed steps
  int result = 0;
  if (Speed > 0) {
    Speed = Speed + 3;                                    // RVVV expects an offset
    result = Speed / 2;                                   // neglect the last bit, thus all uneven speed steps
    bool even = (Speed & 1) == 1;                         // mask / take the last speed bit
    if (even) result = result + 16;                       // If the speed was even, set RVVV bit 4 to 1
  }
  return result;
}

int XpressNetClass::DecodeSpeed(int Speed)
{
  // Derive from the speed byte an integer value (for decoders with 27/28 speed steps)
  int result = 0;                                         // Default speed
  int Bit4 = bitValue(Speed, 4) ? 1 : 0;                  // take bit 4
  int Bit0_3 = Speed % 16;                                // take the lowest 4 bits
  if (Bit0_3 >= 2) {
    result = (Bit0_3 - 2) * 2 + 1 + Bit4;
  }
  return result;
}

// Derive from the speed byte an integer value, according to the speed steps
// 14 and 128 speed steps: 0 = stop, 1 = emergency stop (returned as 0), 2.. = speed step 1..
// 27 and 28 speed steps: DecodeSpeed
int XpressNetClass::DecodeSpeedSteps(int Speed, int Speedsteps)
{
  int value;
  switch (Speedsteps) {
    case 14:
      value = Speed % 16;                                 // take the lowest 4 bits
      if (value >= 2) return value - 1; else return 0;
    case 128:
      value = Speed % 128;                                // take the lowest 7 bits
      if (value >= 2) return value - 1; else return 0;
    default:
      return DecodeSpeed(Speed);
  }
}

// The VB6 Val function: the number at the start of the text, with a point as decimal separator
// (independent of the regional settings). Returns 0 if the text does not start with a number.
// The version of the command station is compared as a number: a string comparison does not work for
// versions >= 10.0
double XpressNetClass::Val(const QString &text)
{
  QString number;
  bool point = false;
  for (QChar c : text.trimmed()) {
    if (c.isDigit()) number += c;
    else if ((c == '.') && !point) { number += c; point = true; }
    else break;
  }
  bool ok = false;
  double result = number.toDouble(&ok);                   // QString::toDouble always uses the C locale (a point)
  if (ok) return result;
  return 0;
}
