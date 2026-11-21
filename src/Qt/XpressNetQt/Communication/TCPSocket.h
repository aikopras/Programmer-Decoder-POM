//
//  TCPSocket.h
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  The TCP connection to the Lenz LAN/USB Interface 23151. See TCPSocket.cpp
//  For internal use by the library.
//

#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QTcpSocket>
#include <QTimer>

class XpressNetClass;

class TCPSocketClass : public QObject
{
  Q_OBJECT

public:
  explicit TCPSocketClass(XpressNetClass *parent);

  void connect(const QString &Host, const QString &Port);
  void closeConnection();
  void SendData(const QByteArray &outBuffer);
  bool connected;                                         // Is the connection established?

private:
  XpressNetClass *Parent;
  QTcpSocket *_socket;
  bool _closed;                                           // closeConnection has been called
  QString _host;                                          // 2.0: for the logfile
  int _port;
  static const int connectTimeOut = 5000;                 // 2.0: ms. Maximum time for a connection attempt
  QTimer connectTimer;                                    // 2.0: stops the connection attempt after connectTimeOut

  void connectTimer_Timer();
  void socketConnected();
  void socketError(QAbstractSocket::SocketError error);
  void socketReadyRead();
  void connectionLost(const QString &description);
  static QString shortDescription(QAbstractSocket::SocketError error, const QString &errorString);
};

#endif // TCPSOCKET_H
