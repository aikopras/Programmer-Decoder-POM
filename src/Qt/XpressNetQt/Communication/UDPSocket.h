//
//  UDPSocket.h
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  The UDP connection to the Z21. See UDPSocket.cpp
//  For internal use by the library.
//

#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <QObject>
#include <QByteArray>
#include <QHostAddress>
#include <QString>
#include <QUdpSocket>

class XpressNetClass;

class UDPSocketClass : public QObject
{
  Q_OBJECT

public:
  explicit UDPSocketClass(XpressNetClass *parent);

  void init(const QString &Host, const QString &Port);
  void close();
  void SendData(const QByteArray &outBuffer);
  bool ready;                                             // Can we send?

private:
  XpressNetClass *Parent;
  QUdpSocket *_socket;
  QHostAddress _remoteAddress;
  quint16 _remotePort;
  bool _closed;                                           // close has been called

  void socketReadyRead();
  void socketError(QAbstractSocket::SocketError error);
};

#endif // UDPSOCKET_H
