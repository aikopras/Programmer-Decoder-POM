//
//  SerialPort.h
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  The USB and serial connections. See SerialPort.cpp
//  For internal use by the library.
//

#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QSerialPort>

class XpressNetClass;

class SerialPortClass : public QObject
{
  Q_OBJECT

public:
  explicit SerialPortClass(XpressNetClass *parent);

  void openPort(const QString &Port, const QString &portSettings, bool Handshake);
  void closePort();
  void SendData(const QByteArray &outBuffer);
  bool isOpen;                                            // Is the port open?

private:
  XpressNetClass *Parent;
  QSerialPort *_port;
  bool _closed;                                           // closePort has been called

  static bool applySettings(QSerialPort *port, const QString &settings);
  void portReadyRead();
  void portError(QSerialPort::SerialPortError error);
};

#endif // SERIALPORT_H
