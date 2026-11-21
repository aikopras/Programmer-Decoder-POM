//
//  PomConnection.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of TCPConnection.m of the macOS version. Sends PoM write and PoM verify messages to the decoder,
//  and receives the answers to PoM verify messages via the RS-Bus.
//  My own decoders react on a PoM verify message by sending the CV value as two RS-Bus feedback messages
//  (RS-Bus address 128): first the low nibble (Nibble = 0), then the high nibble (Nibble = 1).
//
//  Differences with the macOS version:
//  - The XpressNet messages are sent and received by the XpressNetQt library (https://github.com/aikopras/XpressNet),
//    instead of by own code. The library also takes care of the flow control towards the Lenz interface
//    (one command at a time), the retransmissions and the keep-alive.
//  - If the IP addresses for sending PoM messages and for receiving feedback messages are the same, a single
//    XpressNet object (and thus a single TCP connection) is used. Otherwise two objects (and two TCP connections).
//  - PoM write and PoM verify requests are kept in a single queue, and are sent in the order in which they were
//    queued (macOS: two separate queues, sent in ascending CV order). For a 16 bit value the low byte is therefore
//    written first if it is queued first.
//  - If no answer to a PoM verify message is received within 1 second, the message is sent once more.
//    If again no answer is received, CV8 (VID, supported by all decoders) is verified. If the decoder answers,
//    the CV is not implemented; otherwise no decoder listens to this address.
//    (macOS: first the status of both Lenz interfaces is checked, then CV1 and CV8, then the CV is retried.)
//

#include "PomConnection.h"
#include "DccDecoder.h"
#include "CvNames.h"
#include "XpressNet.h"
#include <QSettings>

// ***************************************************************************************************************************************************
// ****************************************************************** Definitions ********************************************************************
// ***************************************************************************************************************************************************
static const int POM_OFFSET = 7000;          // First PoM address starts from this address
static const int RS_BUS_TIMEOUT = 1000;      // Time-out (msec) to receive a RS-Bus feedback response to a PoM verify message
static const int POM_WRITE_DELAY = 50;       // Extra delay (msec) we introduce between PoM write requests
static const int RS_BUS_FEEDBACK_ADDRESS = 128;   // All my decoders send their PoM feedback via this RS-Bus address


PomConnection::PomConnection(DccDecoder *decoder, QObject *parent) : QObject(parent)
{
  this->decoder = decoder;
  xpnPom = nullptr;
  xpnRs = nullptr;
  pomState = Closed;
  rsState = Closed;
  inRun = false;
  errorInRun = false;
  running = false;
  current = {false, 0, 0};
  verifyAttempt = 1;
  probing = false;
  haveFirstNibble = false;
  firstNibble = 0;
  numberOfWriteCvs = 0;
  numberOfVerifyCvs = 0;
  numberOfVerifyReceived = 0;
  rsTimer.setSingleShot(true);
  writeTimer.setSingleShot(true);
  connect(&rsTimer, &QTimer::timeout, this, &PomConnection::rsTimeOut);
  connect(&writeTimer, &QTimer::timeout, this, [this]() {running = false; startNext();});
}

PomConnection::~PomConnection()
{
  deleteConnections();
}


// ****************************************************************************************************************************************************
// ****************************************************************** CONNECT METHODS *****************************************************************
// ****************************************************************************************************************************************************
void PomConnection::open()
{
  deleteConnections();
  createConnections();
}

void PomConnection::openIfClosed()
{
  // Without the PoM connection both connections are opened again
  if (pomState == Closed) {open(); return;}
  // Only the connection for feedbacks is closed. Open it again
  if ((xpnRs != nullptr) && (xpnRs != xpnPom) && (rsState == Closed)) {
    QSettings settings("Programmer Decoder-POM", "Settings");
    rsState = Connecting;
    xpnRs->CmdConnectTcp(settings.value("IpAddressForReceiving").toString());
  }
}

void PomConnection::close()
{
  deleteConnections();
}

void PomConnection::createConnections()
{
  QSettings settings("Programmer Decoder-POM", "Settings");
  QString ipSending = settings.value("IpAddressForSending").toString();
  QString ipReceiving = settings.value("IpAddressForReceiving").toString();
  bool receivingIsActive = settings.value("IpAddressForReceivingIsActive").toBool();
  xpnPom = new XpressNetClass(this);
  connectSignals(xpnPom);
  // Command stations with software version 3.8 or higher support switch addresses up to 2048, older ones up to 1024
  // (the same check as in XpressNetClass::CmdSwitchInfoRequest). The PoM messages go to the sending interface
  connect(xpnPom, &XpressNetClass::ZentraleVersion, this, [this](QString Version) {
    emit maxSwitchAddress((Version.toDouble() < 3.8) ? 1024 : 2048);
  });
  if (!receivingIsActive) xpnRs = nullptr;               // No feedbacks: only PoM write messages are possible
  else if (ipReceiving == ipSending) xpnRs = xpnPom;     // One interface for both
  else {xpnRs = new XpressNetClass(this); connectSignals(xpnRs);}
  pomState = Connecting;
  xpnPom->CmdConnectTcp(ipSending);
  if ((xpnRs != nullptr) && (xpnRs != xpnPom)) {rsState = Connecting; xpnRs->CmdConnectTcp(ipReceiving);}
  else rsState = (xpnRs == nullptr) ? Closed : Connecting;
  emit busy(true);
}

void PomConnection::deleteConnections()
{
  rsTimer.stop();
  writeTimer.stop();
  queue.clear();
  running = false;
  if ((xpnRs != nullptr) && (xpnRs != xpnPom)) delete xpnRs;   // The destructor closes the TCP connection
  delete xpnPom;
  xpnPom = nullptr;
  xpnRs = nullptr;
  pomState = Closed;
  rsState = Closed;
}

void PomConnection::connectSignals(XpressNetClass *xpn)
{
  // Only the errors, warnings and connection messages are logged (in the default directory of the library)
  xpn->LogHexMessages = false;
  xpn->LogDetails = false;
  connect(xpn, &XpressNetClass::InterfaceConnected, this, [this, xpn]() {
    if (xpn == xpnPom) pomState = Connected;
    if (xpn == xpnRs)  rsState = Connected;
    if ((pomState == Connected) && ((xpnRs == nullptr) || (rsState == Connected))) emit generalStatus("Connected to Lenz Interface(s)");
    else if (pomState == Connected) emit generalStatus("Connected to Lenz Interface for Sending PoM messages");
    if (isIdle()) emit busy(false);
    startNext();                                         // Requests may have been queued while connecting
  });
  connect(xpn, &XpressNetClass::TcpError, this, [this, xpn](QString Description) {connectionLost(xpn, Description);});
  connect(xpn, &XpressNetClass::TcpClosedByRemoteHost, this, [this, xpn]() {connectionLost(xpn, "closed by the interface");});
  connect(xpn, &XpressNetClass::SendError, this, [this](quint8, QString Description) {
    emit generalStatus("Lenz interface: " + Description);
  });
  connect(xpn, &XpressNetClass::InterfaceNichtAdressiert, this, [this]() {
    emit generalStatus("Command station no longer connects to interface");
  });
  connect(xpn, &XpressNetClass::InterfaceWiederAdressiert, this, [this]() {
    emit generalStatus("Command station is reconnected to interface");
  });
  // Feedback messages. My own decoders send them as feedback decoder (TT bits = 2), so the library raises Feedback.
  // If a decoder would send them as switch decoder, the library raises SwitchFeedback: we convert the switch address
  // back to the RS-Bus address and nibble (the macOS version does not look at the TT bits)
  connect(xpn, &XpressNetClass::Feedback, this, [this, xpn](int address, quint8 Nibble, quint8 data) {
    if (xpn == xpnRs) handleFeedback(address, Nibble, data);
  });
  connect(xpn, &XpressNetClass::SwitchFeedback, this, [this, xpn](int address, quint8, quint8, quint8 data) {
    if (xpn == xpnRs) handleFeedback(((address - 1) / 4) + 1, ((address - 1) / 2) % 2, data);
  });
}

void PomConnection::connectionLost(XpressNetClass *xpn, const QString &reason)
{
  QString name = (xpn == xpnPom) ? "Lenz interface for sending PoM messages" : "Lenz interface for feedbacks";
  bool wasConnected = ((xpn == xpnPom) ? pomState : rsState) == Connected;
  if (xpn == xpnPom) pomState = Closed;
  if (xpn == xpnRs)  rsState = Closed;
  rsTimer.stop();
  writeTimer.stop();
  queue.clear();
  running = false;
  if (wasConnected) emit generalStatus("Connection to " + name + " lost (" + reason + ")");
    else emit generalStatus("Could not connect to " + name + " (" + reason + ")");
  errorInRun = true;
  finishRun();
}


// ****************************************************************************************************************************************************
// ***************************************************** REQUESTS ARE QUEUED BEFORE TRANSMISSION ******************************************************
// ****************************************************************************************************************************************************
void PomConnection::writeCv(int cvNumber)
{
  queue.enqueue({true, cvNumber, decoder->getCv(cvNumber)});
  startNext();
}

void PomConnection::verifyCv(int cvNumber)
{
  // Without a connection for feedbacks no CV can be verified
  if (xpnRs == nullptr) return;
  // A CV that is already waiting in the queue is not queued twice
  for (const Request &request : queue) if (!request.write && (request.cvNumber == cvNumber)) return;
  queue.enqueue({false, cvNumber, 0});
  startNext();
}

void PomConnection::clearVerifyRequests()
{
  QQueue<Request> writes;
  for (const Request &request : queue) if (request.write) writes.enqueue(request);
  queue = writes;
}

bool PomConnection::isIdle() const
{
  return !running && queue.isEmpty();
}

int PomConnection::pomAddress() const
{
  // We have to add an offset of 7000 to the decoder address to find the loco address
  // Loco address range: 6999 (for initialisation) till 7255
  return decoder->decoderAddress + POM_OFFSET;
}

void PomConnection::startNext()
{
  if (running) return;
  if (queue.isEmpty()) {finishRun(); return;}
  // Wait until the connection(s) are made
  if (pomState == Connecting) return;
  if (!queue.head().write && (rsState == Connecting)) return;
  if (pomState == Closed) {
    queue.clear();
    emit generalStatus("No connection to the Lenz interface for sending PoM messages");
    errorInRun = true;
    finishRun();
    return;
  }
  if (!inRun) {
    // First request of a series: clear the status lines
    inRun = true;
    errorInRun = false;
    emit sendStatus("");
    emit receiveStatus("");
    emit busy(true);
  }
  current = queue.dequeue();
  // Decoder addresses range from 0..255. Note that we accept address = -1, since that address is used for initialisation
  if ((decoder->decoderAddress < -1) || (decoder->decoderAddress > 511)) {
    emit generalStatus("Decoder address should be between 0-511");
    startNext();
    return;
  }
  if ((current.cvNumber < 1) || (current.cvNumber > 1024)) {
    emit generalStatus("CV should be between 1-1024");
    startNext();
    return;
  }
  running = true;
  if (current.write) {
    xpnPom->CmdPomWrite(pomAddress(), current.cvNumber, current.cvValue);
    numberOfWriteCvs++;
    emit sendStatus(QString("POM write message send: CV=%1 Value=%2").arg(current.cvNumber).arg(current.cvValue));
    writeTimer.start(POM_WRITE_DELAY);                  // The timer calls startNext
  }
  else {
    if (rsState != Connected) {running = false; startNext(); return;}   // No feedbacks possible: skip
    verifyAttempt = 1;
    probing = false;
    numberOfVerifyCvs++;
    sendVerify(current.cvNumber);
  }
}

void PomConnection::sendVerify(int cvNumber)
{
  haveFirstNibble = false;
  xpnPom->CmdPomRead(pomAddress(), cvNumber);
  emit sendStatus(QString("POM verify message send: CV=%1").arg(cvNumber));
  rsTimer.start(RS_BUS_TIMEOUT);
}

void PomConnection::finishRun()
{
  if (!inRun) {emit busy(false); return;}
  inRun = false;
  if (numberOfVerifyCvs > 0) {
    if (!errorInRun) emit generalStatus("");
    emit sendStatus(QString("Number of transmitted POM verify messages: %1").arg(numberOfVerifyCvs));
    emit receiveStatus(QString("Number of received POM verify response messages: %1").arg(numberOfVerifyReceived));
  }
  else if (numberOfWriteCvs > 0) {
    emit sendStatus(QString("Number of transmitted POM write messages: %1").arg(numberOfWriteCvs));
  }
  numberOfWriteCvs = 0;
  numberOfVerifyCvs = 0;
  numberOfVerifyReceived = 0;
  emit busy(false);
  emit idle();
}


// ****************************************************************************************************************************************************
// ********************************************************* ANALYZE RS-BUS FEEDBACK MESSAGE **********************************************************
// ****************************************************************************************************************************************************
void PomConnection::handleFeedback(int rsAddress, int nibble, int data)
{
  // Switch and similar decoders send their feedback messages ALWAYS via RS-Address 128
  if (rsAddress != RS_BUS_FEEDBACK_ADDRESS) return;
  if (!running || current.write) return;
  if (nibble == 0) {
    // First nibble (also if a previous first nibble was not followed by a second nibble)
    firstNibble = data & 0x0F;
    haveFirstNibble = true;
    return;
  }
  // Second nibble. If the first nibble was lost, we skip the data and wait for the time-out
  if (!haveFirstNibble) return;
  rsTimer.stop();
  haveFirstNibble = false;
  uint8_t value = ((data & 0x0F) << 4) | firstNibble;
  if (probing) {
    // The decoder answers CV8, thus listens to this address, but does not answer the requested CV
    emit generalStatus(QString("CV %1 not implemented").arg(current.cvNumber));
    errorInRun = true;
  }
  else {
    numberOfVerifyReceived++;
    emit receiveStatus(QString("Feedback received. Address=%1 CV=%2 Value=%3").arg(rsAddress).arg(current.cvNumber).arg(value));
    emit cvReceived(current.cvNumber, value);           // May queue new requests; these are sent below
  }
  running = false;
  startNext();
}

void PomConnection::rsTimeOut()
{
  haveFirstNibble = false;
  if (probing) {
    // Also CV8 is not answered: no decoder listens to this address
    if (decoder->decoderAddress < 0) emit generalStatus("No non-initialised decoders found");
      else emit generalStatus(QString("No decoder responds to address: %1").arg(decoder->decoderAddress));
    errorInRun = true;
    clearVerifyRequests();
    running = false;
    startNext();
    return;
  }
  if (verifyAttempt == 1) {
    // May be a temporary problem, like the loss of a RS-Bus message. Try a second time
    verifyAttempt = 2;
    sendVerify(current.cvNumber);
    return;
  }
  // Second time-out: check with CV8 whether a decoder listens to this address
  probing = true;
  sendVerify(CV::VID);
}
