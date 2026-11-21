//
//  UDPSocket.cpp
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  The UDP connection to the Z21 (VB6: UdpClass with the Winsock control).
//  A port of UDPSocket.cs of the .NET version, with a QUdpSocket. The QUdpSocket delivers its signals in the thread
//  of the XpressNet object (the event loop of the application), so no thread transitions are needed.
//
//  As in VB6, the local port is selected by the operating system (bind to port 0). The Z21 spec prefers port 21106
//  for the client, but with a fixed local port only a single instance of the UDP class would be possible.
//  Note: the Yamorc YD7001D retransmits broadcast messages three times.
//

#include "UDPSocket.h"
#include "XpressNet.h"
#include "XpressNetReceiveZ21.h"
#include "XpressNetLogFile.h"
#include <QHostInfo>
#include <QNetworkDatagram>
#include <QNetworkProxy>


UDPSocketClass::UDPSocketClass(XpressNetClass *parent) : QObject(parent)
{
  Parent = parent;
  _socket = nullptr;
  _remotePort = 0;
  _closed = false;
  ready = false;
}


// ==================================================================================================================
void UDPSocketClass::init(const QString &Host, const QString &Port)
{
  bool ok = false;
  int portNumber = Port.trimmed().toInt(&ok);
  if (!ok || (portNumber < 1) || (portNumber > 65535)) {
    Parent->Logfile->Errors("UDP Error: invalid port: " + Port);
    return;
  }
  // The address of the Z21: an IP address, or a host name (IPv4 only)
  QHostAddress address;
  if (!address.setAddress(Host.trimmed())) {
    bool found = false;
    QHostInfo info = QHostInfo::fromName(Host.trimmed());
    for (const QHostAddress &a : info.addresses()) {
      if (a.protocol() == QAbstractSocket::IPv4Protocol) {
        address = a;
        found = true;
        break;
      }
    }
    if (!found) {
      Parent->Logfile->Errors("UDP Error: no IPv4 address for host: " + Host);
      return;
    }
  }
  _remoteAddress = address;
  _remotePort = quint16(portNumber);
  _socket = new QUdpSocket(this);
  // The Z21 is in the local network: never via a proxy of the system (as the VB6 and .NET versions)
  _socket->setProxy(QNetworkProxy::NoProxy);
  if (!_socket->bind(QHostAddress::AnyIPv4, 0)) {         // Let the operating system select a random free port
    ready = false;
    Parent->Logfile->Errors("UDP Error: " + _socket->errorString());
    return;
  }
  QObject::connect(_socket, &QUdpSocket::readyRead, this, &UDPSocketClass::socketReadyRead);
  QObject::connect(_socket, &QUdpSocket::errorOccurred, this, &UDPSocketClass::socketError);
  ready = true;
  // 2.0: which connection belongs to the letter of this XpressNet object
  Parent->Logfile->Connection(Host + ", port " + QString::number(portNumber) + ", local port " + QString::number(_socket->localPort()));
}

void UDPSocketClass::close()
{
  _closed = true;
  ready = false;
  if (_socket != nullptr) _socket->close();
}

void UDPSocketClass::SendData(const QByteArray &outBuffer)
{
  if (!ready) return;
  if (_socket->writeDatagram(outBuffer, _remoteAddress, _remotePort) < 0) {
    Parent->Logfile->Errors("UDP Error while sending: " + _socket->errorString());
  }
}


// ==================================================================================================================
// Signals of the QUdpSocket
void UDPSocketClass::socketReadyRead()
{
  // The application may close the connection from an event handler (_closed); the remaining datagrams are ignored
  while ((!_closed) && _socket->hasPendingDatagrams()) {
    QNetworkDatagram datagram = _socket->receiveDatagram();
    if (datagram.isValid()) Parent->receiveZ21->addToIndata(datagram.data());
  }
}

// For example: an ICMP "port unreachable" of a previous send. The socket remains usable, so we continue receiving
void UDPSocketClass::socketError(QAbstractSocket::SocketError error)
{
  Q_UNUSED(error);
  if (_closed) return;
  Parent->Logfile->Details("UDP: " + _socket->errorString());
}
