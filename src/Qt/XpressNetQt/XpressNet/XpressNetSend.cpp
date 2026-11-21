//
//  XpressNetSend.cpp
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of the SendClass of the VB6 XpressNet DLL: implementation of the message and nextMessage methods.
//  A port of XpressNetSend.cs of the .NET version.
//
//  message is called by the main XpressNet code to send (prepare the sending of) an XpressNet message.
//  The parameters to the message method are the individual bytes of the XpressNet message, separated
//  by comma's. These bytes should NOT include the two Framing bytes at the beginning (FF and FE), nor
//  the XOR byte at the end.
//
//  nextMessage is called from the ReceiveClass as soon as a previous message has been acknowledged by the
//  command station. nextMessage doesn't need any parameters, but instead takes (pop) the oldest XpressNet
//  message that is still waiting from the FIFO queue.
//
//  A FIFO queue is needed, since XpressNet implements flow control between PC and the Command Station.
//  The Command Station acknowledges the reception of every individual message to the PC.
//  The PC can send the next message only after the acknowledgement of the previous message is received,
//  or a time-out (of 5 seconds during normal operation, or 90 seconds during programming) has occured.
//  The FIFO allows the main program to "send" multiple XpressNet messages at once, and thus relieves
//  the main program from any concerns regarding flow control.
//  The Z21 (UDP) has no flow control: the Z21 does not answer every command (Z21 spec, section 1.1),
//  therefore Z21 messages are sent immediately.
//
//  Version 2.0 (November 2026). The functionality below is the same in the VB6, .NET, macOS and Qt versions:
//  - Retransmission. If the interface answers a message with an error that may disappear if the message is sent
//    again, the message is sent again: at most 3 times, after 100, 200 and 400 msec. These answers are:
//    61 80 (Ubertragungsfehler), 61 81 (Zentrale Busy), 01 01, 01 02, 01 03, 01 06 and 01 0A (V3.6: 1.5 and 3.1.5;
//    for 01 0A the specification advises to repeat the last command). If the last retransmission fails as well, the
//    message is given up: logfile and event SendError, and the next message from the FIFO is sent.
//    The answers 01 09 (Fehler in den Befehlsparametern) and 61 82 (Befehl in Zentrale nicht vorhanden) are not
//    solved by a retransmission: logfile and event SendError, and the next message is sent.
//  - Pause. After 01 05 (Zentrale adressiert das Interface nicht mehr) or 01 08 (Derzeit konnen keine Befehle an
//    die Zentrale gesendet werden) the interface can not pass messages to the command station. Sending is paused
//    (event InterfaceNichtAdressiert); the FIFO is kept. With 01 08 the last message did not reach the command
//    station: it is held, and sent first after the pause. After 01 07 (Zentrale adressiert das Interface wieder)
//    sending continues (event InterfaceWiederAdressiert). If 01 07 does not come within 5 sec (90 sec in
//    programming mode), both queues are emptied (event SendError) and the pause ends.
//    During the pause, messages for the interface itself (header F0, F1, F2; for example the TCP keep alive
//    message) are still sent: these are answered by the interface, and not by the command station.
//  - Nothing is dropped silently. A message that is not sent gives a line in the logfile and the event SendError.
//  - Retransmission and pause are not used for the Z21 (UDP): without flow control it is not known to which message
//    an answer belongs.
//
//  Differences with the VB6 version (the same as the .NET version):
//  - Header bytes for XpressNet / Z21 are added here, in transmit (VB6: in the classes Tcp, Udp and Usb)
//  - As long as the connection is not (yet) established, messages are dropped (VB6: only without connection)
//  - A byte value outside 0..255 is not sent, but written to the logfile (VB6: run time error "Overflow")
//  The timers are QTimer objects (Qt::PreciseTimer). The keep alive timer is periodic; the other timers fire once
//  (single shot).
//

#include "XpressNetSend.h"
#include "XpressNetAccAuto.h"
#include "XpressNetLogFile.h"
#include "TCPSocket.h"
#include "UDPSocket.h"
#include "SerialPort.h"

static inline QString str(int value) { return QString::number(value); }


SendClass::SendClass(XpressNetClass *parent)
{
  Parent = parent;
  waitForAck = false;                                     // The start value is that sending data is allowed
  emptyFifo();
  // The timers are precise (Qt::PreciseTimer): a coarse timer (the default) may be up to 5% early or late
  lanKeepAliveTimer.setTimerType(Qt::PreciseTimer);
  tcpResponseTimer.setTimerType(Qt::PreciseTimer);
  retryTimer.setTimerType(Qt::PreciseTimer);
  pauseTimer.setTimerType(Qt::PreciseTimer);
  // Timer for TCP (and UDP) keep alive messages
  QObject::connect(&lanKeepAliveTimer, &QTimer::timeout, &lanKeepAliveTimer, [this]() { lanKeepAliveTimer_Timer(); });
  lanKeepAliveTimer.start(lanKeepAliveInterval);
  waitForlanKeepAliveResponse = false;                    // We did not yet send a LAN Keep Alive message
  // Timer for TCP response time outs. The timer will be started as soon as a message is send
  tcpResponseTimer.setSingleShot(true);
  QObject::connect(&tcpResponseTimer, &QTimer::timeout, &tcpResponseTimer, [this]() { tcpResponseTimer_Timer(); });
  // 2.0: retransmission and pause
  retryTimer.setSingleShot(true);                         // The timer will be started after an error answer
  QObject::connect(&retryTimer, &QTimer::timeout, &retryTimer, [this]() { retryTimer_Timer(); });
  pauseTimer.setSingleShot(true);                         // The timer will be started at the begin of a pause
  QObject::connect(&pauseTimer, &QTimer::timeout, &pauseTimer, [this]() { pauseTimer_Timer(); });
  retryCount = 0;
  retryPending = false;
  interfaceNotAddressed = false;
}

// Stops the timers
SendClass::~SendClass()
{
  lanKeepAliveTimer.stop();
  tcpResponseTimer.stop();
  retryTimer.stop();
  pauseTimer.stop();
}


// ------------------------------------------------------------------------------------------------------------------
// message is called from the XpressNetClass code. It either sends a message, or it stores the message
// in the FIFO. If a message is send, it calles transmit, which in turn activates the flow control
// by setting the waitForAck Flag to True.
// Note: Header bytes for Xpressnet / Z21 are, if needed, added in transmit
// 2.0: a message that is not sent gives the event SendError (besides the line in the logfile). The checks of
// the message are done first, so the description of the event can include the XOR byte
void SendClass::message(std::initializer_list<int> byteSequence)
{
  // STEP 1: Perform 3 checks on the size of the received message.
  // Check 1: did we receive any input??
  if (byteSequence.size() == 0) {
    notSent(FehlerInDenBefehlsparametern, "Error in the message subroutine: ParamArray is empty", byteSequence);
    return;
  }
  // Check 2: Is the input not too long?
  if (int(byteSequence.size()) > maxMessageSize) {
    notSent(FehlerInDenBefehlsparametern, "Error in the message subroutine: ParamArray exceeds maxFifoMsgSize", byteSequence);
    return;
  }
  // Check 3: Does the received number of bytes correspond to the XpressNet message header?
  // The number of bytes received must match with the length as contained in (the lower 4 bits) of the first
  // (=header) byte. Note: the Z21 Specification sometimes violates this, and must be excluded from this check
  if ((!Parent->Z21Protocol) && ((int(byteSequence.size()) - 1) != (*byteSequence.begin() % 16))) {
    notSent(FehlerInDenBefehlsparametern, "Error in the message subroutine: size ParamArray doesn't match size in header", byteSequence);
    return;
  }
  // Extra check: every byte must have a value between 0 and 255 (VB6: run time error "Overflow")
  for (int value : byteSequence) {
    if ((value < 0) || (value > 255)) {
      notSent(FehlerInDenBefehlsparametern, "Error in the message subroutine: value " + str(value) + " does not fit in a byte", byteSequence);
      return;
    }
  }
  // STEP 2: Create the XpressNet message, including the XOR value
  QByteArray outBuffer;
  quint8 xorValue = 0;
  for (int value : byteSequence) {
    outBuffer.append(char(value));
    xorValue ^= quint8(value);
  }
  outBuffer.append(char(xorValue));
  // STEP 3: Check if we have already a connection. If not: drop the packet
  // 2.0: also without any connection (connectionType none) the logfile and the event SendError (1.3: silently)
  if (Parent->connectionType == none) {
    notSent(KeineVerbindung, "Message not sent: there is no connection", outBuffer);
    return;
  }
  if (!Parent->isConnected()) {
    notSent(KeineVerbindung, "Message not sent: the connection has not (yet) been established", outBuffer);
    return;
  }
  // STEP 4: Check if we can send immediately, or if the message should be stored in the FIFO
  // 2.0: during a pause (01 05 / 01 08) only messages for the interface itself are sent (header F0, F1, F2)
  bool interfaceCommand = (quint8(outBuffer[0]) >= 0xF0);
  if ((waitForAck == false) && ((!interfaceNotAddressed) || interfaceCommand)) {
    transmit(outBuffer);
  }
  else {                                                  // Store the message in the FIFO
    // Check if the FIFO queue has free slots left to store the message
    // If not, increase "maxFifoEntries", or modify the calling program
    if (fifo.size() >= maxFifoEntries) {                  // FIFO overflow :-(
      notSent(FifoVoll, "Error: FIFO queue is full", outBuffer);
      return;
    }
    fifo.enqueue(outBuffer);
  }
}


// ------------------------------------------------------------------------------------------------------------------
// nextMessage will be called from the ReceiveClass code. Provided that we are allowed to send now,
// it sends a message from the FIFO queue, if there are still messages waiting.
// 2.0: not during a pause. After a pause the held message (01 08) is sent first
void SendClass::nextMessage()
{
  if ((waitForAck == false) && (!interfaceNotAddressed)) {   // we may send (no flow control, no pause)
    if (!heldMessage.isEmpty()) {
      QByteArray held = heldMessage;
      heldMessage.clear();
      transmit(held, true);                               // the held message keeps its number of retransmissions
    }
    // Check if the FIFO has messages waiting. If yes, send the oldest message from the FIFO
    else if (fifo.size() > 0) {
      transmit(fifo.dequeue());
    }
  }
}

// 2.0: retransmission is true if byteArray is sent again (the number of retransmissions is kept)
void SendClass::transmit(const QByteArray &byteArray, bool retransmission)
{
  // 2.0: keep the message, for a retransmission
  if (!retransmission) retryCount = 0;
  lastMessage = byteArray;
  // No flow control for the Z21 (UDP). The Z21 does not answer every command (Z21 spec 1.1: communication is
  // asynchronous; many commands have no reply), so waiting for an answer could block the FIFO.
  // Messages for the Z21 are therefore sent immediately; the FIFO is not used for UDP
  if (Parent->connectionType != lanUdpPort) waitForAck = true;   // We may not send a next message immediately
  QByteArray outBuffer;
  int dataLen;
  switch (Parent->connectionType) {                       // Select output
    case lanTcpPort:
      outBuffer = addHeader(byteArray, {0xFF, 0xFE});     // Two Framing bytes
      if (Parent->LogHexMessages) Parent->Logfile->SendHex(outBuffer);
      lanKeepAliveTimer.start(lanKeepAliveInterval);      // Restart the timer
      if (Parent->tcp != nullptr) Parent->tcp->SendData(outBuffer);
      break;
    case lanUdpPort:
      // The four bytes for the XpressNet tunnel: DataLen (2 bytes, little endian), Header 0x40 0x00
      dataLen = byteArray.size() + 4;
      outBuffer = addHeader(byteArray, {quint8(dataLen % 256), quint8(dataLen / 256), 0x40, 0x00});
      if (Parent->LogHexMessages) Parent->Logfile->SendHex(outBuffer);
      lanKeepAliveTimer.start(lanKeepAliveInterval);      // Restart the timer
      if (Parent->udp != nullptr) Parent->udp->SendData(outBuffer);
      break;
    case usbPort:
      outBuffer = addHeader(byteArray, {0xFF, 0xFE});     // Two Framing bytes
      if (Parent->LogHexMessages) Parent->Logfile->SendHex(outBuffer);
      if (Parent->usb != nullptr) Parent->usb->SendData(outBuffer);
      break;
    case serialPort:                                      // Serial interfaces have no Framing bytes
      if (Parent->LogHexMessages) Parent->Logfile->SendHex(byteArray);
      if (Parent->usb != nullptr) Parent->usb->SendData(byteArray);
      break;
    default:
      break;
  }
  // The response timer is used for TCP, USB and serial. The USB interface has the same rule as LAN: no next
  // command before the response, or before the timeout (5 sec, 1.5 min in programming mode)
  // (LI-USB: 1.1, Interface 23151: 1.3). Serial interfaces may omit the confirmation 1/4/5 (LI101F: 1.4).
  // Without this timer, a missing response would block the FIFO.
  if (Parent->connectionType != lanUdpPort) {
    if (Parent->ZentraleProgrammierMode) {                // Responses in programming mode may take 90 sec
      tcpResponseTimer.start(tcpProgrammingTimeOut);
    }
    else {
      tcpResponseTimer.start(tcpResponseTimeOut);         // In normal mode the response must be within 5 sec
    }
  }
  Parent->NumberOfExpressNetMessagesTransmitted = Parent->NumberOfExpressNetMessagesTransmitted + 1;
}

// 2.0: sends a Z21 LAN message that is not an XpressNet message (no tunnel header 0x40, no XOR), such as
// LAN_SET_BROADCASTFLAGS and LAN_LOGOFF. z21Message contains the complete message, including DataLen and Header.
// As for the XpressNet messages to the Z21: no flow control and no FIFO. The message is not counted in
// NumberOfExpressNetMessagesTransmitted, since it is not an XpressNet message
void SendClass::transmitZ21(const QByteArray &z21Message)
{
  if ((Parent->connectionType != lanUdpPort) || (Parent->udp == nullptr)) {
    notSent(KeineVerbindung, "Message not sent: there is no Z21 (UDP) connection", z21Message);
    return;
  }
  if (Parent->LogHexMessages) Parent->Logfile->SendHex(z21Message);
  lanKeepAliveTimer.start(lanKeepAliveInterval);          // Restart the timer
  Parent->udp->SendData(z21Message);
}

QByteArray SendClass::addHeader(const QByteArray &message, std::initializer_list<quint8> header)
{
  QByteArray result;
  for (quint8 value : header) result.append(char(value));
  result.append(message);
  return result;
}

void SendClass::lanKeepAliveTimer_Timer()
{
  // V3.6: 2.1 (Interface Status Befehl - Keep Alive)
  // For LAN interface, send keep alive message, to inform the interface we are still listening
  // This is required for the TCP (XpressNet) as well as the UDP (Z21) connection
  // 2.0: not while the TCP connection is being established (the message would give the event SendError)
  if (!Parent->isConnected()) return;
  if (Parent->connectionType == lanTcpPort) {
    waitForlanKeepAliveResponse = true;
    Parent->CmdInterfaceStatus();                         // This short message is used as XpressNet "Keep Alive"
    Parent->Logfile->DetailsSent("TCP Keep Alive Request");
  }
  if (Parent->connectionType == lanUdpPort) {
    waitForlanKeepAliveResponse = true;
    Parent->CmdStatusZentraleAnfordern();                 // This short message seems usable as Z21 "Keep Alive"
    Parent->Logfile->DetailsSent("UDP Keep Alive Request");
  }
}

void SendClass::tcpResponseTimer_Timer()
{
  // For TCP, USB and serial: if the timer expires, something went wrong. Let's go to a well-defined state
  // The keep alive message is for TCP only (Interface 23151: chapter 2 is for the LAN interface only)
  if (Parent->connectionType != lanUdpPort) {
    tcpResponseTimer.stop();                              // Stop the response timer
    // 2.0: the description for the event SendError (before lastMessage changes)
    QString description = "No answer within " + str(tcpResponseTimer.interval() / 1000) + " sec (message: " + hex(lastMessage) + ")";
    int removed = fifo.size();
    if (!heldMessage.isEmpty()) removed = removed + 1;
    emptyFifo();                                          // Remove all XpressNet messages still waiting
    heldMessage.clear();
    retryTimer.stop();                                    // 2.0: no retransmission is waiting
    retryPending = false;
    retryCount = 0;
    waitForAck = false;                                   // Clear the message acknowledgement flag
    // Remove all switch commands still waiting. 2.0: the activated switch is switched off first. waitForAck is
    // cleared before, so this message is sent immediately
    int removedAccessories = Parent->AccAuto->emptyFifo();
    if (Parent->connectionType == lanTcpPort) {
      waitForlanKeepAliveResponse = true;                 // We will send a keep alive message
      Parent->CmdInterfaceStatus();                       // We will send a keep alive message
    }
    Parent->Logfile->Errors("Error: TCP Response Timer has timed out. Message queues emptied");
    description = description + ". Message queues emptied (" + str(removed) + " messages, " + str(removedAccessories) + " CmdAccessoryAuto commands)";
    Parent->eventSendError(Timeout, description);
  }
}

void SendClass::stoptcpResponseTimer()
{
  // Will be called from the receive class as soon as a response is received
  tcpResponseTimer.stop();
}

int SendClass::size()
{
  return fifo.size();
}

void SendClass::emptyFifo()
{
  fifo.clear();
}


// ------------------------------------------------------------------------------------------------------------------
// 2.0: RETRANSMISSION (see the header of this file)
// Called from the ReceiveClass if the interface answers the last message with an error that may disappear if the
// message is sent again: 61 80, 61 81, 01 01, 01 02, 01 03, 01 06 and 01 0A. answer is the text for the logfile
void SendClass::answerError(const QString &answer)
{
  if (Parent->connectionType == lanUdpPort) return;       // Z21: no flow control (see the header)
  if (lastMessage.isEmpty()) return;                      // No message has been sent yet
  if (retryCount < maxRetries) {
    int delay = firstRetryDelay << retryCount;            // 100, 200, 400 msec
    retryCount = retryCount + 1;
    retryPending = true;
    waitForAck = true;                                    // The FIFO waits for the retransmission
    retryTimer.start(delay);
    Parent->Logfile->Warnings("Answer " + answer + ": retransmission " + str(retryCount) + " of " + str(maxRetries) + " after " + str(delay) + " msec (message: " + hex(lastMessage) + ")");
  }
  else {
    retryCount = 0;
    QString description = "Message given up after " + str(maxRetries) + " retransmissions. Answer: " + answer + " (message: " + hex(lastMessage) + ")";
    Parent->Logfile->Errors(description);
    Parent->eventSendError(WiederholungErfolglos, description);
    // waitForAck is false: the next message is sent by nextMessage (ReceiveClass::DecodeXpressNetMessage, STEP 4)
  }
}

// Called from the ReceiveClass if the interface answers the last message with 01 09 or 61 82. A retransmission
// does not help: the message is given up, and the next message is sent. reason: FehlerInDenBefehlsparametern (01 09)
// or BefehlNichtVorhanden (61 82)
void SendClass::answerRejected(const QString &answer, SendErrorReasonEnum reason)
{
  if (Parent->connectionType == lanUdpPort) return;       // Z21: no flow control (see the header)
  if (lastMessage.isEmpty()) return;                      // No message has been sent yet
  retryCount = 0;
  QString description = "Message rejected. Answer: " + answer + " (message: " + hex(lastMessage) + ")";
  Parent->Logfile->Errors(description);
  Parent->eventSendError(reason, description);
}

void SendClass::retryTimer_Timer()
{
  if (!retryPending) return;
  retryPending = false;
  if (interfaceNotAddressed) {                            // A pause started in the meantime: send after the pause
    heldMessage = lastMessage;
    waitForAck = false;                                   // No message is underway
    return;
  }
  transmit(lastMessage, true);
}


// ------------------------------------------------------------------------------------------------------------------
// 2.0: PAUSE (see the header of this file)
// Called from the ReceiveClass after 01 05 (answer = false) or 01 08 (answer = true). With 01 08 the last
// message did not reach the command station: it is held, and sent first after the pause
void SendClass::pause(bool answer)
{
  if (Parent->connectionType == lanUdpPort) return;       // Z21: no flow control (see the header)
  if (answer && (!lastMessage.isEmpty())) heldMessage = lastMessage;
  if (interfaceNotAddressed) return;                      // The pause has started already
  interfaceNotAddressed = true;
  if (Parent->ZentraleProgrammierMode) {                  // In programming mode the pause may take 90 sec
    pauseTimer.start(pauseProgrammingTimeOut);
  }
  else {
    pauseTimer.start(pauseTimeOut);
  }
  Parent->Logfile->Warnings("Sending paused, until the command station addresses the interface again");
  Parent->eventInterfaceNichtAdressiert();
}

// Called from the ReceiveClass after 01 07. The held message is sent first, then the FIFO and CmdAccessoryAuto.
// The event is raised last, so messages the application sends from the event handler come after these
void SendClass::resume()
{
  if (Parent->connectionType == lanUdpPort) return;       // Z21: no flow control (see the header)
  if (!interfaceNotAddressed) return;                     // 01 07 without a pause (for example after power on): ignored
  interfaceNotAddressed = false;
  pauseTimer.stop();
  Parent->Logfile->Warnings("Sending resumed: the command station addresses the interface again");
  nextMessage();                                          // First the held message, otherwise the FIFO
  Parent->AccAuto->resume();                              // CmdAccessoryAuto continues
  Parent->eventInterfaceWiederAdressiert();
}

void SendClass::pauseTimer_Timer()
{
  if (!interfaceNotAddressed) return;
  int seconds = pauseTimer.interval() / 1000;
  interfaceNotAddressed = false;                          // The pause ends
  int removed = fifo.size();
  if (!heldMessage.isEmpty()) removed = removed + 1;
  emptyFifo();                                            // Remove all XpressNet messages still waiting
  heldMessage.clear();
  retryCount = 0;
  if (retryPending) {                                     // No retransmission anymore
    retryTimer.stop();
    retryPending = false;
    waitForAck = false;
  }
  // Remove all switch commands still waiting. The activated switch (if any) is switched off first
  int removedAccessories = Parent->AccAuto->emptyFifo();
  QString description = "The command station did not address the interface again within " + str(seconds) + " sec. Message queues emptied (" + str(removed) + " messages, " + str(removedAccessories) + " CmdAccessoryAuto commands)";
  Parent->Logfile->Errors(description);
  Parent->eventSendError(ZentraleAdressiertDasInterfaceNichtMehr, description);
}


// ------------------------------------------------------------------------------------------------------------------
// 2.0: A message that is not sent: logfile and event SendError. The description contains the bytes of the message
void SendClass::notSent(SendErrorReasonEnum reason, const QString &text, const QByteArray &message)
{
  writeLog(reason, text);
  Parent->eventSendError(reason, text + " (message: " + hex(message) + ")");
}

void SendClass::notSent(SendErrorReasonEnum reason, const QString &text, std::initializer_list<int> byteSequence)
{
  writeLog(reason, text);
  QString s;
  for (int value : byteSequence) {
    if (s.length() > 0) s.append(" ");
    s.append(QString("%1").arg(value, 2, 16, QChar('0')).toUpper());
  }
  Parent->eventSendError(reason, text + " (message: " + s + ")");
}

// The same log level as in version 1.3: a missing connection is a warning, the rest an error
void SendClass::writeLog(SendErrorReasonEnum reason, const QString &text)
{
  if (reason == KeineVerbindung) Parent->Logfile->Warnings(text);
  else Parent->Logfile->Errors(text);
}

// The bytes of a message as text, for example "52 00 88 DA"
QString SendClass::hex(const QByteArray &message)
{
  QString s;
  for (char value : message) {
    if (s.length() > 0) s.append(" ");
    s.append(QString("%1").arg(quint8(value), 2, 16, QChar('0')).toUpper());
  }
  return s;
}
