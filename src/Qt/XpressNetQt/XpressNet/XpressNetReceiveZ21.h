//
//  XpressNetReceiveZ21.h
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of the ReceiveClassZ21 of the VB6 XpressNet DLL. See XpressNetReceiveZ21.cpp
//  For internal use by the library.
//

#ifndef XPRESSNETRECEIVEZ21_H
#define XPRESSNETRECEIVEZ21_H

#include "XpressNet.h"
#include <QByteArray>

class ReceiveClassZ21
{
public:
  explicit ReceiveClassZ21(XpressNetClass *parent);

  void addToIndata(const QByteArray &inBytes);

private:
  XpressNetClass *Parent;

  // The buffer below is used to store the input data received from the UDP socket.
  static const int inDataSize = 1478;                     // Maximum size of the UDP Data (Section 1.3 Z21 spec)
  quint8 inData[inDataSize + 1];                          // Buffer to store received bytes (VB6: inData(inDataSize))
  int indexNextInByte;                                    // Index into the indata Array. Empty space to store subsequent byte
  bool busy;                                              // Are the messages in inData being processed? (see ReceiveClass)

  void checkInDataForZ21messages();
  void DecodeZ21Message(int Z21length, const quint8 *Z21Message);
};

#endif // XPRESSNETRECEIVEZ21_H
