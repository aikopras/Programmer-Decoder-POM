//
//  XpressNetReceiveZ21.cpp
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of the ReceiveClassZ21 of the VB6 XpressNet DLL. A port of XpressNetReceiveZ21.cs of the .NET version.
//  This class implements the Z21 receive code for XpressNet messages that are transferred "within"
//  Z21 messages (the Z21 Xpressnet tunnel).
//  The first 4 bytes of these massages are: DataLen (2 bytes) 0x40 0x00.
//  The remaining bytes are the XpressNet message.
//  Note that the last four bits of the XpressNet header do not always include the correct
//  XpressNet message length. Therefore we must use the Z21 DataLen
//  For Z21 details, see the Z21 LAN Protocol Specification (Version 1.13, 06.11.2023)
//
//  According to section 1.3 of the Z21 protocol spec, multiple Z21 messages may be combined
//  within a single UDP packet.
//  Although the Z21 specification doesn't say anything regarding retransmission, it seems
//  that at least the Yamorc YD7001D command station retransmits broadcast messages.
//  Probably that is done since UDP is, by nature, unreliable.
//

#include "XpressNetReceiveZ21.h"
#include "XpressNetReceive.h"
#include "XpressNetLogFile.h"
#include <algorithm>


ReceiveClassZ21::ReceiveClassZ21(XpressNetClass *parent)
{
  Parent = parent;
  indexNextInByte = 0;                                    // the first empty space in the inData array.
  busy = false;
  for (int i = 0; i <= inDataSize; i++) inData[i] = 0;
}

void ReceiveClassZ21::addToIndata(const QByteArray &inBytes)
{
  for (int i = 0; i < inBytes.size(); i++) {              // include all bytes that we received as input
    if (indexNextInByte > inDataSize) {                   // avoid buffer overflow
      Parent->Logfile->Errors("Error: overflow Z21 receive input buffer");
      return;
    }
    inData[indexNextInByte] = quint8(inBytes[i]);
    indexNextInByte = indexNextInByte + 1;
  }
  // Now that new data is stored, check if the inBuffer contains one or more
  // Z21 messages. If yes, process this / these message(s)
  if (busy) return;                                       // the loop that is still running processes the new bytes
  busy = true;
  checkInDataForZ21messages();
  busy = false;
}

void ReceiveClassZ21::checkInDataForZ21messages()
{
  // Check if inData contains one or more complete Z21 messages
  int Z21length;                                          // the length of the Z21 message (from the first two bytes)
  int i;
  // Since we may have multiple Z21/XpressNet message within the inData buffer, we have a loop
  // that ends whenever the inData buffer is empty (indexnextInByte points to 0).
  // We use "return" to leave the loop, whenever the inData buffer contains "garbage")
  while (indexNextInByte > 0) {
    // Step 1: Determine the length of the Z21 and XpressNet messages. DataLen consists of 2 bytes (little endian)
    Z21length = inData[0] + (inData[1] * 256);
    // Step 2: Check if we have at least this number of bytes in the indata buffer
    // If not, we have something inconsistent in the inData buffer. A DataLen below 4 is wrong as well.
    // In such case we raise an error, clear the indata buffer and exit the subroutine
    if ((Z21length > indexNextInByte) || (Z21length < 4)) {
      Parent->Logfile->Errors("Error: Z21 messagesize exceeds the number of received UDP bytes");
      indexNextInByte = 0;                                // clear the inBuffer
      return;                                             // and return
    }
    // 2.0: log every received Z21 message, also the messages that are not decoded (such as other headers than
    // 0x40, or messages longer than 23 bytes). 1.3: only the decoded messages. At most the first 256 bytes
    if (Parent->LogHexMessages) Parent->Logfile->ReceiveHexRange(inData, 0, std::min(Z21length - 1, 255));
    // STEP 3: Copy the Z21 message in a new buffer
    // Z21Message can hold 23 bytes. Longer Z21 messages are not copied (and not decoded).
    // The XpressNet messages are shorter (LAN_X_LOCO_INFO: at most 21 bytes)
    quint8 Z21Message[23] = {0};                          // array to store the Z21 Message (VB6: Z21Message(22))
    bool copied = (Z21length <= int(sizeof(Z21Message)));
    if (copied) {
      for (i = 0; i < Z21length; i++) {
        Z21Message[i] = inData[i];
      }
    }
    // STEP 4: Remove the z21 message from the inData buffer, and shift the remainder to the begin
    i = 0;
    for (int j = Z21length; j <= (indexNextInByte - 1); j++) {
      inData[i] = inData[j];
      i = i + 1;
    }
    indexNextInByte = i;                                  // points to the next empty byte
    // STEP 5: Call the subroutine that analyses the individual Z21 message (only if it has been copied)
    if (copied) DecodeZ21Message(Z21length, Z21Message);
  }
}

void ReceiveClassZ21::DecodeZ21Message(int Z21length, const quint8 *Z21Message)
{
  int Xpnlength;                                          // the length of the XpressNet message (without the XOR byte)
  int indexXor;                                           // points to the last byte of the XpressNet message (=XOR byte)
  quint8 xorValue;                                        // to calculate the XOR over all bytes in the XpressNet message
  // STEP 1: Exit if the Z21Message does not contain an XpressNet message
  if (Z21Message[2] != 0x40) return;
  // An XpressNet message has at least an X-Header and a XOR byte (DataLen >= 6)
  if (Z21length < 6) return;
  // STEP2: Retrieve the length of the XpressNet message from the Z21 DataLen
  Xpnlength = Z21length - 5;                              // Xpnlength is without the XOR byte
  indexXor = Z21length - 1;                               // Since the first index is 0, we should subtract 1
  // STEP 3: Should we log the XpressNet packet?
  // 2.0: the message has already been logged in checkInDataForZ21messages
  // if (Parent->LogHexMessages) Parent->Logfile->ReceiveHexRange(Z21Message, 0, indexXor);
  // STEP 4: Check if the XOR value is correct
  xorValue = 0;                                           // Initialise
  for (int i = 4; i <= indexXor; i++) {                   // If we include the XOR byte in the XOR calculation
    xorValue ^= Z21Message[i];                            // the result should become 0
  }
  if (xorValue != 0) {                                    // XOR error. Exit sub
    Parent->Logfile->Errors("XOR Error in received XpressNet message");
    if (Parent->LogErrors) Parent->Logfile->ReceiveHexRange(Z21Message, 0, indexXor);
    return;
  }
  // NOTE: in the "normal receive routine we check if the received message is a broadcast message
  // We could do that here as well, but since there is no flow control, knowing if something is a
  // broadcast message doesn;t bring anything
  // STEP 5: Copy the XpressNet message into the xpnMessage buffer
  quint8 XpnMessage[19] = {0};                            // array to store the XpressNet Message (VB6: XpnMessage(18))
  for (int i = 0; i <= Xpnlength; i++) {
    XpnMessage[i] = Z21Message[i + 4];
  }
  // STEP 6: Call the subroutine that analyses the individual XpressNet message
  Parent->receive->DecodeXpressNetMessage(quint8(Xpnlength), XpnMessage, false);
}
