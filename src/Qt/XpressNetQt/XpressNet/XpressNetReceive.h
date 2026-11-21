//
//  XpressNetReceive.h
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of the ReceiveClass of the VB6 XpressNet DLL. See XpressNetReceive.cpp
//  For internal use by the library.
//

#ifndef XPRESSNETRECEIVE_H
#define XPRESSNETRECEIVE_H

#include "XpressNet.h"
#include <QByteArray>

class ReceiveClass
{
public:
  explicit ReceiveClass(XpressNetClass *parent);

  void addToIndata(const QByteArray &inBytes);
  void DecodeXpressNetMessage(quint8 length, const quint8 *message, bool broadcast);

private:
  XpressNetClass *Parent;

  // The buffer below is used to store "raw" input data from the LAN/USB/Serial interfaces
  // Everytime bytes are received from a physical (LAN/USB/Serial) interface, the event handler
  // for that interface calls addToIndata to store the received bytes within inData
  static const int inDataSize = 1024;
  quint8 inData[inDataSize + 1];                          // Buffer to store received bytes (VB6: inData(inDataSize))
  int indexNextInByte;                                    // Index into the indata Array. Empty space to store subsequent byte
  bool busy;                                              // Are the messages in inData being processed?

  void processInData();
  void checkInDataForXpressNetmessages();
  void removeUntilNextFrame();                            // 2.0
  void AnswerFromInterface(quint8 data, bool broadcast);
  void DecodeFeedback(quint8 length, const quint8 *message);
};

#endif // XPRESSNETRECEIVE_H
