//
//  TCPSocket.cpp
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  The TCP connection to the Lenz LAN/USB Interface 23151 (VB6: TcpClass with the Winsock control).
//  A port of TCPSocket.cs of the .NET version, with a QTcpSocket. The QTcpSocket delivers its signals in the thread
//  of the XpressNet object (the event loop of the application), so no thread transitions are needed.
//
//  Events:
//  - connection established          => Parent->eventInterfaceConnected
//  - connection could not be made    => Parent->eventTcpError (with the description of the error)
//  - connection closed by the remote host, or lost  => Parent->eventTcpClosedByRemoteHost
//  - data received                   => Parent->receive->addToIndata
//
//  2.0: a connection attempt is stopped after 5 seconds (the operating system itself may wait much longer, for
//  example after a typing error in the IP address). The event TcpError gets a short description (as the
//  Winsock control in VB6); the logfile gets the complete description of the error
//
//  Note: this class has a method connect (as the .NET version). Within this class, the Qt function is therefore
//  called as QObject::connect
//

#include "TCPSocket.h"
#include "XpressNet.h"
#include "XpressNetReceive.h"
#include "XpressNetLogFile.h"
#include <QNetworkProxy>


TCPSocketClass::TCPSocketClass(XpressNetClass *parent) : QObject(parent)
{
  Parent = parent;
  _socket = nullptr;
  _closed = false;
  connected = false;
  _port = 0;
  connectTimer.setSingleShot(true);
  connectTimer.setTimerType(Qt::PreciseTimer);
  QObject::connect(&connectTimer, &QTimer::timeout, this, &TCPSocketClass::connectTimer_Timer);
}


// ==================================================================================================================
void TCPSocketClass::connect(const QString &Host, const QString &Port)
{
  bool ok = false;
  int portNumber = Port.trimmed().toInt(&ok);
  if (!ok || (portNumber < 1) || (portNumber > 65535)) {
    Parent->eventTcpError("Invalid TCP port: " + Port);
    return;
  }
  _host = Host;
  _port = portNumber;
  _socket = new QTcpSocket(this);
  // The interface is in the local network: never via a proxy of the system (as the VB6 and .NET versions)
  _socket->setProxy(QNetworkProxy::NoProxy);
  QObject::connect(_socket, &QTcpSocket::connected, this, &TCPSocketClass::socketConnected);
  QObject::connect(_socket, &QTcpSocket::errorOccurred, this, &TCPSocketClass::socketError);
  QObject::connect(_socket, &QTcpSocket::readyRead, this, &TCPSocketClass::socketReadyRead);
  QObject::connect(_socket, &QTcpSocket::disconnected, this, [this]() { connectionLost(""); });
  connectTimer.start(connectTimeOut);
  _socket->connectToHost(Host, quint16(portNumber));
}

// 2.0: the connection attempt takes too long
void TCPSocketClass::connectTimer_Timer()
{
  if (_closed || connected) return;
  closeConnection();                                      // The result of the connection attempt is ignored
  Parent->eventTcpError("Connection timed out", "Connection attempt timed out after " + QString::number(connectTimeOut / 1000) + " seconds");
}

// 2.0: a short description for the event TcpError (as the Winsock control in VB6)
QString TCPSocketClass::shortDescription(QAbstractSocket::SocketError error, const QString &errorString)
{
  switch (error) {
    case QAbstractSocket::ConnectionRefusedError: return "Connection refused";
    case QAbstractSocket::SocketTimeoutError: return "Connection timed out";
    case QAbstractSocket::HostNotFoundError: return "Host not found";
    default: return errorString;                          // For example "Network unreachable" or "Host unreachable"
  }
}

void TCPSocketClass::closeConnection()
{
  _closed = true;
  connected = false;
  connectTimer.stop();
  if (_socket != nullptr) _socket->abort();               // The connection is closed immediately
}

void TCPSocketClass::SendData(const QByteArray &outBuffer)
{
  // STEP 1: Do we still have the TCP connection?
  if (!connected) {
    Parent->eventTcpClosedByRemoteHost();                 // Clean up, and inform the application
    return;
  }
  // STEP 2: Send the outbuffer. flush sends the bytes immediately (without waiting for the event loop)
  if (_socket->write(outBuffer) < 0) {
    Parent->Logfile->Errors("TCP Error while sending: " + _socket->errorString());
    connected = false;
    Parent->eventTcpClosedByRemoteHost();                 // Clean up, and inform the application
    return;
  }
  _socket->flush();
}


// ==================================================================================================================
// Signals of the QTcpSocket
void TCPSocketClass::socketConnected()
{
  if (_closed) return;
  connectTimer.stop();
  connected = true;
  // XpressNet messages are small, and should be sent immediately. The option can only be set once the socket exists
  _socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
  // 2.0: which connection belongs to the letter of this XpressNet object
  Parent->Logfile->Connection(_host + ", port " + QString::number(_port) + ", local port " + QString::number(_socket->localPort()));
  Parent->eventInterfaceConnected();
}

void TCPSocketClass::socketError(QAbstractSocket::SocketError error)
{
  if (_closed) return;
  if (!connected) {                                       // The connection attempt failed
    connectTimer.stop();
    Parent->eventTcpError(shortDescription(error, _socket->errorString()), _socket->errorString());
    return;
  }
  // The connection has been closed by the remote host, or has been lost
  if (error == QAbstractSocket::RemoteHostClosedError) connectionLost("");
  else connectionLost(_socket->errorString());
}

void TCPSocketClass::socketReadyRead()
{
  QByteArray tcpData = _socket->readAll();
  if (_closed) return;
  if (tcpData.size() > 0) Parent->receive->addToIndata(tcpData);
}

void TCPSocketClass::connectionLost(const QString &description)
{
  if (_closed) return;                                    // We closed the connection ourselves
  if (description.length() > 0) Parent->Logfile->Errors("TCP Error while receiving: " + description);
  connected = false;
  if (Parent->tcp == this) Parent->eventTcpClosedByRemoteHost();
}
