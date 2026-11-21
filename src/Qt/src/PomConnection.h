//
//  PomConnection.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  See PomConnection.cpp
//

#ifndef POMCONNECTION_H
#define POMCONNECTION_H

#include <QObject>
#include <QQueue>
#include <QTimer>
#include <cstdint>

class XpressNetClass;
class DccDecoder;

class PomConnection : public QObject
{
  Q_OBJECT

public:
  explicit PomConnection(DccDecoder *decoder, QObject *parent = nullptr);
  ~PomConnection() override;

  void open();                            // Opens the connection(s), with the IP addresses of the preferences
  void openIfClosed();                    // Opens the connection(s) that are closed. Does nothing if all are open
  void close();
  void writeCv(int cvNumber);             // Queues a PoM write; the value is taken from the decoder object now
  void verifyCv(int cvNumber);            // Queues a PoM verify; the answer comes via the RS-Bus (address 128)
  void clearVerifyRequests();             // Removes the PoM verify requests from the queue
  bool isIdle() const;                    // Nothing queued and nothing being sent

signals:
  void cvReceived(int cvNumber, uint8_t cvValue);
  void idle();                            // All queued requests have been handled
  void generalStatus(const QString &text);
  void sendStatus(const QString &text);
  void receiveStatus(const QString &text);
  void busy(bool active);
  void maxSwitchAddress(int address);    // 1024 or 2048, depending on the software version of the command station

private:
  struct Request {
    bool write;                           // true: PoM write. false: PoM verify
    int cvNumber;
    uint8_t cvValue;                      // Only for PoM write
  };

  DccDecoder *decoder;
  XpressNetClass *xpnPom;                 // Connection for sending PoM messages
  XpressNetClass *xpnRs;                  // Connection for RS-Bus feedback. Same object as xpnPom if the IP addresses are equal
                                          // nullptr if no feedback interface is active
  enum ConnectionState {Closed, Connecting, Connected};
  ConnectionState pomState;
  ConnectionState rsState;                // Closed if xpnRs is nullptr
  QQueue<Request> queue;
  bool inRun;                             // A series of requests is being handled (for the status lines)
  bool errorInRun;                        // An error message was shown during this series
  bool running;                           // A request is being handled
  Request current;
  // PoM verify
  int verifyAttempt;                      // 1: first attempt, 2: retry
  bool probing;                           // We verify CV8, to check whether a decoder listens to this address
  bool haveFirstNibble;
  uint8_t firstNibble;
  QTimer rsTimer;                         // Time-out for the RS-Bus feedback
  QTimer writeTimer;                      // Delay between PoM write messages
  // Counters for the status lines
  int numberOfWriteCvs;
  int numberOfVerifyCvs;
  int numberOfVerifyReceived;

  void createConnections();
  void deleteConnections();
  void connectSignals(XpressNetClass *xpn);
  int pomAddress() const;
  void startNext();
  void sendVerify(int cvNumber);
  void finishRun();
  void rsTimeOut();
  void handleFeedback(int rsAddress, int nibble, int data);
  void connectionLost(XpressNetClass *xpn, const QString &reason);
};

#endif // POMCONNECTION_H
