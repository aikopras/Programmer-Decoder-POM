//
//  XpressNetSend.h
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of the SendClass of the VB6 XpressNet DLL. See XpressNetSend.cpp
//  For internal use by the library.
//

#ifndef XPRESSNETSEND_H
#define XPRESSNETSEND_H

#include "XpressNet.h"
#include <QByteArray>
#include <QQueue>
#include <QTimer>
#include <initializer_list>

class SendClass
{
public:
  explicit SendClass(XpressNetClass *parent);
  ~SendClass();                                           // Stops the timers

  bool waitForAck;                                        // Can we send the message now, or should it be stored in the FIFO?
  bool waitForlanKeepAliveResponse;                       // Flag that helps to distinguish where certain responses belong to
  bool interfaceNotAddressed;                             // 2.0: Is sending paused (01 05 / 01 08)? Also read by AccAutoClass

  void message(std::initializer_list<int> byteSequence);
  void nextMessage();
  void transmit(const QByteArray &byteArray, bool retransmission = false);
  void transmitZ21(const QByteArray &z21Message);         // 2.0
  void stoptcpResponseTimer();
  int size();
  void emptyFifo();
  // 2.0: retransmission and pause
  void answerError(const QString &answer);
  void answerRejected(const QString &answer, SendErrorReasonEnum reason);
  void pause(bool answer);
  void resume();
  static QString hex(const QByteArray &message);

private:
  XpressNetClass *Parent;

  // If needed, we store XpressNet messages in a FIFO. Each element holds 1 XpressNet message, including the XOR
  static const int maxFifoEntries = 128;                  // The number of XpressNet messages the queue is able to hold
  static const int maxMessageSize = 11;                   // Maximum number of bytes of an XpressNet message from PC to
                                                          // Command Station (without XOR). As in VB6: maxFifoMsgSize - 1
  QQueue<QByteArray> fifo;

  // Timers
  static const int lanKeepAliveInterval = 30000;          // Time between LAN Keep Alive messages
  QTimer lanKeepAliveTimer;
  static const int tcpResponseTimeOut = 5000;             // A response message should be received within 5 sec
  static const int tcpProgrammingTimeOut = 90000;         // In programming mode a response may take 90 sec
  QTimer tcpResponseTimer;

  // 2.0: Retransmission (see the header of XpressNetSend.cpp)
  QByteArray lastMessage;                                 // The last transmitted message (including the XOR byte)
  int retryCount;                                         // The number of retransmissions of lastMessage
  bool retryPending;                                      // A retransmission of lastMessage waits for the retryTimer
  static const int maxRetries = 3;                        // At most 3 retransmissions
  static const int firstRetryDelay = 100;                 // The delays are 100, 200 and 400 msec
  QTimer retryTimer;

  // 2.0: Pause (see the header of XpressNetSend.cpp)
  QByteArray heldMessage;                                 // The message that is sent first after the pause (01 08)
  static const int pauseTimeOut = 5000;                   // 01 07 should be received within 5 sec
  static const int pauseProgrammingTimeOut = 90000;       // In programming mode within 90 sec
  QTimer pauseTimer;

  void lanKeepAliveTimer_Timer();
  void tcpResponseTimer_Timer();
  void retryTimer_Timer();
  void pauseTimer_Timer();
  static QByteArray addHeader(const QByteArray &message, std::initializer_list<quint8> header);
  void notSent(SendErrorReasonEnum reason, const QString &text, const QByteArray &message);
  void notSent(SendErrorReasonEnum reason, const QString &text, std::initializer_list<int> byteSequence);
  void writeLog(SendErrorReasonEnum reason, const QString &text);
};

#endif // XPRESSNETSEND_H
