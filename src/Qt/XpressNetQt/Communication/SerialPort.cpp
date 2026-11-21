//
//  SerialPort.cpp
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  The USB and serial connections (VB6: UsbClass with the MSComm control).
//  A port of SerialPort.cs of the .NET version, with a QSerialPort. The QSerialPort delivers its signals in the
//  thread of the XpressNet object (the event loop of the application), so no thread transitions are needed.
//
//  Info regarding Lenz interfaces (from the VB6 version):
//  1) Serial:
//  - LI100: First design. Metal case. Baudrate: 9600.
//  - LI100F: Improved design. Metal case. Baudrates: 9600 (default) and 19200 baud.
//  - LI101F: Last version of the serial interface. Plastic case. Galvanic isolation
//            Baudrates: 19200 (default), 38400, 57600, en 115200 baud.
//  All serial interfaces use:
//  - 8 Databits, 1 Start bit, 1 Stop bit, No parity => Settings = "baudrate,N,8,1"
//  - Hardware-Handshake: the interface uses CTS (Clear To Send) to tell the PC whether or not
//    it is able to receive data. The interface ignores the PC's RTS (Request to Send) signal.
//    Note that the Interface's DSR (Data Set Ready) and the PC's DTR (Data terminal Ready) are not used.
//  2) USB
//  - LI-USB (23150), LI-USB-Ethernet (23151), LZV200, Yamorc YD7001
//  All Lenz USB interfaces use:
//  - 57600 Baud, 8 Databits, 1 Start bit, 1 Stop bit, No parity => Settings = "57600,N,8,1"
//  - No hardware handshake, according to the Lenz documentation (LI-USB 06/2008, Interface 23151 V1.2).
//    2.0: JMRI uses hardware handshake (RTS/CTS) for the LI-USB 23150. Therefore CmdConnectUsb has the optional
//    parameter Handshake (default: off), just like CmdConnectSerial.
//  The Yamorc YD7001 USB interface seems to work with 57600, as well as 115200.
//
//  Received bytes may contain parts of an XpressNet message, or (parts of) multiple XpressNet messages.
//  ReceiveClass::addToIndata takes care of this.
//
//  Port is the name of the serial device: on Linux for example /dev/ttyUSB0 (FTDI, such as the LI-USB) or /dev/ttyACM0,
//  on macOS for example /dev/cu.usbserial-..., on Windows for example COM3. On Linux the user must be allowed to use
//  the device (normally: member of the group dialout).
//  If the port can not be opened, the event TcpError is raised (VB6 and .NET: a MessageBox; macOS: TcpError as well).
//  If the device is removed (USB cable disconnected), the port is closed: messages then give the event SendError
//  (KeineVerbindung).
//

#include "SerialPort.h"
#include "XpressNet.h"
#include "XpressNetReceive.h"
#include "XpressNetLogFile.h"
#include <QStringList>


SerialPortClass::SerialPortClass(XpressNetClass *parent) : QObject(parent)
{
  Parent = parent;
  _port = nullptr;
  _closed = false;
  isOpen = false;
}


// ==================================================================================================================
void SerialPortClass::openPort(const QString &Port, const QString &portSettings, bool Handshake)
{
  _port = new QSerialPort(this);
  _port->setPortName(Port);
  if (!applySettings(_port, portSettings)) {
    isOpen = false;
    Parent->eventTcpError("Could not open " + Port + " (invalid settings: " + portSettings + ")");
    return;
  }
  if (Handshake) {
    _port->setFlowControl(QSerialPort::HardwareControl);  // Serial uses CTS for hardware handshaking
  }
  else {
    _port->setFlowControl(QSerialPort::NoFlowControl);    // No hardware handshaking
  }
  if (!_port->open(QIODevice::ReadWrite)) {               // Try to open the USB port
    // Catch errors if the port does not exist
    isOpen = false;
    Parent->eventTcpError("Could not open " + Port + " (" + _port->errorString() + ")");
    return;
  }
  if (!Handshake) _port->setRequestToSend(false);         // The Lenz XpressNet spec says it ignores RTS
  _port->setDataTerminalReady(false);                     // DSR/DTR is not implemented by Lenz
  QObject::connect(_port, &QSerialPort::readyRead, this, &SerialPortClass::portReadyRead);
  QObject::connect(_port, &QSerialPort::errorOccurred, this, &SerialPortClass::portError);
  isOpen = true;
  // 2.0: which connection belongs to the letter of this XpressNet object
  Parent->Logfile->Connection(Port + ", " + portSettings + (Handshake ? ", handshake" : ""));
  Parent->eventInterfaceConnected();
}

void SerialPortClass::closePort()
{
  _closed = true;
  isOpen = false;
  if ((_port != nullptr) && _port->isOpen()) _port->close();
}

void SerialPortClass::SendData(const QByteArray &outBuffer)
{
  if (!isOpen) return;
  // flush sends the bytes immediately (without waiting for the event loop)
  if (_port->write(outBuffer) < 0) {
    Parent->Logfile->AppendWithHost("COM Error: " + _port->errorString());
    return;
  }
  _port->flush();
}

// Settings as in the MSComm control: "baudrate,parity,databits,stopbits", for example "57600,N,8,1"
bool SerialPortClass::applySettings(QSerialPort *port, const QString &settings)
{
  QStringList parts = settings.split(',');
  if (parts.size() != 4) return false;
  bool ok = false;
  int baudRate = parts[0].trimmed().toInt(&ok);
  if (!ok || (baudRate <= 0)) return false;
  port->setBaudRate(baudRate);
  QString parity = parts[1].trimmed().toUpper();
  if (parity == "N") port->setParity(QSerialPort::NoParity);
  else if (parity == "E") port->setParity(QSerialPort::EvenParity);
  else if (parity == "O") port->setParity(QSerialPort::OddParity);
  else if (parity == "M") port->setParity(QSerialPort::MarkParity);
  else if (parity == "S") port->setParity(QSerialPort::SpaceParity);
  else return false;
  QString dataBits = parts[2].trimmed();
  if (dataBits == "5") port->setDataBits(QSerialPort::Data5);
  else if (dataBits == "6") port->setDataBits(QSerialPort::Data6);
  else if (dataBits == "7") port->setDataBits(QSerialPort::Data7);
  else if (dataBits == "8") port->setDataBits(QSerialPort::Data8);
  else return false;
  QString stopBits = parts[3].trimmed();
  if (stopBits == "1") port->setStopBits(QSerialPort::OneStop);
  else if (stopBits == "1.5") port->setStopBits(QSerialPort::OneAndHalfStop);
  else if (stopBits == "2") port->setStopBits(QSerialPort::TwoStop);
  else return false;
  return true;
}


// ==================================================================================================================
// Signals of the QSerialPort
void SerialPortClass::portReadyRead()
{
  QByteArray usbData = _port->readAll();
  if (_closed) return;
  if (usbData.size() > 0) Parent->receive->addToIndata(usbData);
}

void SerialPortClass::portError(QSerialPort::SerialPortError error)
{
  if (_closed) return;
  if ((error == QSerialPort::NoError) || (error == QSerialPort::TimeoutError)) return;
  Parent->Logfile->AppendWithHost("COM Error: " + _port->errorString());
  // The device has been removed (for example: the USB cable has been disconnected). The port is closed
  if (error == QSerialPort::ResourceError) closePort();
}
