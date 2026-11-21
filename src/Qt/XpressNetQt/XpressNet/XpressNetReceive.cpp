//
//  XpressNetReceive.cpp
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of the ReceiveClass of the VB6 XpressNet DLL. A port of XpressNetReceive.cs of the .NET version.
//  Receives the bytes from the TCP, USB and serial connections, splits these into individual XpressNet messages,
//  and decodes each XpressNet message. DecodeXpressNetMessage is also used for the Z21 (ReceiveClassZ21).
//
//  The various methods contain references to the V3.6 and V4 XpressNet specifications:
//  - Befehlsbeschreibung XpressNet V3.6 mit LAN/USB Interface 23151
//    Kommunikation auf den Schnittstellen, Doku Version 1.2 05/2019
//    https://www.lenz-elektronik.de/src/pdf/23151_Info.zip
//  - Dokumentation XpressNet Version 4.0, 02/2022:
//    https://www.lenz-elektronik.de/src/pdf/Lenz_XpressNet_Doku.pdf
//  For details, see these documents
//
//  Version 2.0 (November 2026), the same in the VB6, .NET, macOS and Qt versions:
//  - The answers of the interface to a message (V3.6: 1.5 and 3.1.5) are passed to the SendClass, which sends the
//    message again (retransmission), gives up the message, or pauses sending (see XpressNetSend.cpp).
//  - 01 05 and 01 07 are always handled as broadcast (unrequested), also for a serial interface without framing bytes
//  - After a message with an XOR error, the next framing bytes (FF FE or FF FD) are searched (TCP and USB). Without
//    this, a lost byte could also disturb the next messages. Serial interfaces have no framing bytes.
//

#include "XpressNetReceive.h"
#include "XpressNetSend.h"
#include "XpressNetLogFile.h"


ReceiveClass::ReceiveClass(XpressNetClass *parent)
{
  Parent = parent;
  indexNextInByte = 0;                                    // the first empty space in the inData array.
  busy = false;
  for (int i = 0; i <= inDataSize; i++) inData[i] = 0;
}

// Note: a slot of the application may show a QMessageBox (a local event loop). During that time, new received
// bytes are passed to addToIndata, while the previous bytes are still being processed (busy). The new bytes are
// then only stored; the loop in checkInDataForXpressNetmessages that is still running processes them.
void ReceiveClass::addToIndata(const QByteArray &inBytes)
{
  for (int i = 0; i < inBytes.size(); i++) {              // include all bytes that we received as input
    // Some command stations (like the YD7001) may generate quite some data
    // If the buffer is full, first process the complete XpressNet messages, and then continue.
    // An XpressNet message is at most 19 bytes (2 framing bytes, header, 15 data bytes, XOR), so afterwards the
    // buffer contains at most 18 bytes.
    if (indexNextInByte > inDataSize) {
      if (busy) {
        Parent->Logfile->Errors("Error: overflow receive input buffer");
        return;
      }
      processInData();
    }
    inData[indexNextInByte] = quint8(inBytes[i]);
    indexNextInByte = indexNextInByte + 1;
  }
  // Now that new data is stored, check if the inBuffer contains one or more
  // complete XpressNet messages. If yes, process this / these message(s)
  if (!busy) processInData();
}

void ReceiveClass::processInData()
{
  busy = true;
  checkInDataForXpressNetmessages();
  busy = false;
}

void ReceiveClass::checkInDataForXpressNetmessages()
{
  // Check if inData contains one or more complete XpressNet messages
  int indexHeader;                                        // points to the XpressNet header
  int indexXor;                                           // points to the last byte of the XpressNet message (=XOR byte)
  int Xpnlength;                                          // the length of the XpressNet message (from the header byte)
  quint8 xorValue;                                        // to calculate the XOR over all bytes in the XpressNet message
  bool broadcast;                                         // is the received message a broadcast? (FF FD)
  int i;
  // Since we may have multiple XpressNet message within the inData buffer, we have a loop
  // that doesn't end. We use "return" to leave the loop, whenever the inData buffer is
  // empty or contains an incomplete XpressNet message
  while (true) {
    // Step 1: Determine where the XpressNet header is located
    if (inData[0] == 0xFF) indexHeader = 2;
    else indexHeader = 0;
    // Exit, if the header byte has not yet been received
    if (indexNextInByte <= indexHeader) return;
    // STEP2: Retrieve the length of the XpressNet message from the last four header bits
    Xpnlength = inData[indexHeader] % 16;                 // Mod 16 gives us the last four bits
    indexXor = indexHeader + Xpnlength + 1;               // length excludes the XOR
    // Exit, if we have not yet received the last byte of an XpressNet message
    if (indexNextInByte <= indexXor) return;
    // STEP 3: We have a complete XpressNet packet. Should we log it?
    if (Parent->LogHexMessages) Parent->Logfile->ReceiveHexRange(inData, 0, indexXor);
    // STEP 4: Check if the XOR value is correct
    xorValue = 0;                                         // Initialise
    for (i = indexHeader; i <= indexXor; i++) {           // If we include the XOR byte in the XOR calculation
      xorValue ^= inData[i];                              // the result should become 0
    }
    if (xorValue == 0) {                                  // No XOR error. XpressNet message is OK
      // STEP 5: Is this a broadcast message?
      broadcast = (inData[0] == 0xFF) && (inData[1] == 0xFD);
      // STEP 6: Copy the XpressNet message into the xpnMessage buffer
      quint8 XpnMessage[19] = {0};                        // array to store the XpressNet Message (VB6: XpnMessage(18))
      i = 0;
      for (int j = indexHeader; j <= (indexHeader + Xpnlength); j++) {
        XpnMessage[i] = inData[j];
        i = i + 1;
      }                                                   // At the end, i holds the length of the XpressNet message
      // STEP 7: Call the subroutine that analyses the individual XpressNet message
      DecodeXpressNetMessage(quint8(i), XpnMessage, broadcast);
    }
    else {                                                // XOR Error
      Parent->Logfile->Errors("XOR Error in received XpressNet message");
      // 2.0: up to and including the XOR byte (1.3: Xpnlength + 1, without the last two bytes of a framed message)
      if (Parent->LogErrors) Parent->Logfile->ReceiveHexRange(inData, 0, indexXor);
      // 2.0: TCP and USB: remove the bytes up to the next framing bytes, and continue with that message
      if ((Parent->connectionType == lanTcpPort) || (Parent->connectionType == usbPort)) {
        removeUntilNextFrame();
        continue;
      }
    }
    // STEP 8 Remove the old message from the inData buffer, and shift the remainder to the begin
    // Garbage is removed here as well (with a XOR error), since a "message" is at most 19 bytes
    i = 0;
    for (int j = (indexXor + 1); j <= (indexNextInByte - 1); j++) {
      inData[i] = inData[j];
      i = i + 1;
    }
    indexNextInByte = i;                                  // points to the next empty byte
  }
}

// 2.0: After an XOR error: remove the bytes before the next framing bytes (FF FE or FF FD). The search starts at
// the second byte, since the first byte belongs to the message with the XOR error. Without framing bytes the buffer
// is emptied, except a last byte FF (this may be the first framing byte of the next message)
void ReceiveClass::removeUntilNextFrame()
{
  int next = indexNextInByte;                             // By default: remove everything
  for (int j = 1; j < indexNextInByte; j++) {
    if (inData[j] == 0xFF) {
      if (j == (indexNextInByte - 1)) {                   // A last byte FF: keep it
        next = j;
        break;
      }
      if ((inData[j + 1] == 0xFE) || (inData[j + 1] == 0xFD)) {
        next = j;
        break;
      }
    }
  }
  int i = 0;
  for (int j = next; j <= (indexNextInByte - 1); j++) {
    inData[i] = inData[j];
    i = i + 1;
  }
  indexNextInByte = i;                                    // points to the next empty byte
}

void ReceiveClass::DecodeXpressNetMessage(quint8 length, const quint8 *message, bool broadcast)
{
  // This routine is called to analyse an individual XpressNet message
  // The first byte should be the XpressNet header; framing bytes should already be removed
  // XOR has already been checked and the XOR byte is removed.
  // STEP 1: Extract from the XpressNet header byte the message length as well as the main command type
  quint8 header = message[0];
  int msgCmd = header / 16;                               // The main command type in the 4 highest order bits
  // STEP 2: Set the flow-control flag, except when the message was a broadcast
  // 2.0: 01 05 and 01 07 are always unrequested (V3.6: 1.5, framing bytes FF FD). Also for a serial interface,
  // which has no framing bytes: without this check, these would release the flow control
  if ((header == 0x01) && ((message[1] == 0x05) || (message[1] == 0x07))) broadcast = true;
  if (broadcast == false) {
    Parent->send->waitForAck = false;
    Parent->send->stoptcpResponseTimer();
  }
  // STEP 3: Call the procedure that is specific for the message received
  // Check first for feedback messages, which have as Command = 4 but varying length
  if (msgCmd == 4) {
    DecodeFeedback(length, message);
  }
  else {
    // All other message have a fixed length, so we can do a Case statement on the entire header
    switch (header) {
      case 0x01: AnswerFromInterface(message[1], broadcast); break;
      case 0x02: Parent->eventInterfaceVersion(message[1], message[2]); break;
      case 0x61:
        switch (message[1]) {
          case 0x00: Parent->eventNotAus(); break;
          case 0x01: Parent->eventAllesAn(); break;
          case 0x02: Parent->eventProgrammierMode(); break;
          case 0x08: Parent->eventProgrammierinfoKurzschluss(); break;  // Z21 LAN_X_BC_TRACK_SHORT_CIRCUIT
          case 0x11: Parent->eventProgrammierinfoZentraleBereit(); break;
          case 0x12: Parent->eventProgrammierinfoKurzschluss(); break;
          case 0x13: Parent->eventProgrammierinfoDatenNichtGefunden(); break;
          case 0x1F: Parent->eventProgrammierinfoZentraleBusy(); break;
          // 2.0: the answer is passed to the SendClass (retransmission or SendError)
          case 0x80:
            Parent->Logfile->Errors("<= Ubertragungsfehler");
            if (!broadcast) Parent->send->answerError("61 80 (Ubertragungsfehler)");
            break;
          case 0x81:
            Parent->Logfile->Errors("<= Zentrale Busy");
            if (!broadcast) Parent->send->answerError("61 81 (Zentrale Busy)");
            break;
          case 0x82:
            Parent->Logfile->Warnings("<= Befehl in Zentrale nicht vorhanden");
            if (!broadcast) Parent->send->answerRejected("61 82 (Befehl in Zentrale nicht vorhanden)", BefehlNichtVorhanden);
            break;
        }
        break;
      case 0x62:
        if (message[1] == 0x22) Parent->eventStatusZentrale(message[2]);
        break;
      case 0x63:
        switch (message[1]) {
          case 0x03: Parent->eventModellZeit(message[2], message[3]); break;
          case 0x10: Parent->eventProgInfoPageRegister(message[2], message[3]); break;
          case 0x14: Parent->eventProgInfo(1, message[2], message[3]); break;   // CV1-255 u. 1024
          case 0x15: Parent->eventProgInfo(2, message[2], message[3]); break;   // CV256 - 511
          case 0x16: Parent->eventProgInfo(3, message[2], message[3]); break;   // CV512 - 767
          case 0x17: Parent->eventProgInfo(4, message[2], message[3]); break;   // CV768 - 1023
          case 0x20: Parent->eventServiceVariableMelden(message[2], message[3]); break;
          case 0x21: Parent->eventSoftwareVersionZentrale(message[2], message[3]); break;
        }
        break;
      case 0x64:
        switch (message[1]) {
          case 0x14: Parent->eventProgInfoZ21(message[2], message[3], message[4]); break;
          case 0x24: Parent->eventPomErgebnisMelden(message[2], message[3], message[4]); break;
          case 0x25: Parent->eventModellZeit(message[2], message[3], message[4]); break;
        }
        break;
      case 0x67:
        if (message[1] == 0x23) {
          Parent->eventErweiterteVersionZentrale(message[2], message[3], message[4], message[5], message[6], message[7]);
        }
        break;
      case 0x81:
        if (message[1] == 0x00) Parent->eventNotHalt();
        break;
      case 0xE1: Parent->eventFehlermeldung(message[1]); break;
      case 0xE2: break;                                   // MTR-Basisadresse
      case 0xE3:                                          // Lok-Suchergebnis
        switch (message[1]) {
          case 0x40: break;                               // Lok besetzt
          case 0x50: Parent->eventFunctionStatusF0F12(message[2], message[3]); break;
          case 0x52: Parent->eventFunctionsZustandF13F20(message[2], message[3]); break;
        }
        break;
      case 0xE4:
        if (message[1] == 0x51) {
          Parent->eventFunctionStatusF13F20(message[2], message[3], message[4]);
        }
        else {
          Parent->eventNormaleLokinfo(message[1], message[2], message[3], message[4]);
        }
        break;
      case 0xE5: break;                                   // MTR-Mitglied
      case 0xE6:
        switch (message[1]) {
          case 0x53: Parent->eventFunctionsZustandF29F68(message[2], message[3], message[4], message[5], message[6]); break;
          case 0x54: Parent->eventFunctionStatusF29F68(message[2], message[3], message[4], message[5], message[6]); break;
          default: break;                                 // Lok ist in DTR
        }
        break;
      case 0xEF:
        Parent->eventZ21Lokinfo(message[1], message[2], message[3], message[4], message[5], message[6], message[7], message[8]);
        break;
      case 0xF2:
        switch (message[1]) {
          case 0x01: Parent->eventXpressNetAddress(message[2]); break;
          case 0x02: Parent->eventXpressNetVersion(message[2]); break;
          case 0x03: Parent->eventInterfaceFreieVerbindungen(message[2]); break;
        }
        break;
      case 0xF3:
        switch (message[1]) {
          case 0x0A: Parent->eventz21FirmwareVersion(message[2], message[3]); break;
        }
        break;
    }
  }
  // STEP 4: Try to send the next message that is waiting in the queue
  // Flow control (check WaitForAck) is performed within nextmessage()
  Parent->send->nextMessage();
  // STEP 5: Update the counter for the number of received messages
  Parent->NumberOfExpressNetMessagesReceived = Parent->NumberOfExpressNetMessagesReceived + 1;
}

// ==================================================================================================================
// V3.6: Section 1.5 - Antworten des Interface 23151
// 2.0: the answers are passed to the SendClass: retransmission (01 01, 02, 03, 06, 0A), SendError (01 09),
// pause (01 05, 01 08) or end of the pause (01 07). An error answer that comes as broadcast is only logged
void ReceiveClass::AnswerFromInterface(quint8 data, bool broadcast)
{
  switch (data) {
    case 1:
      Parent->Logfile->Errors("<= Interface: Die im Header angegebene Anzahl Bytes stimmt nicht mit der Anzahl der empfangenen Bytes uberein");
      if (!broadcast) Parent->send->answerError("01 01 (Anzahl Bytes stimmt nicht)");
      break;
    case 2:
      Parent->Logfile->Errors("<= Interface: Fehler zwischen Interface und Zentrale (Timeout bei Datenubertragung des Interface an die Zentrale)");
      if (!broadcast) Parent->send->answerError("01 02 (Fehler zwischen Interface und Zentrale)");
      break;
    case 3:
      Parent->Logfile->Errors("<= Interface: Nnbekannter Fehler (Zentrale adressierte das Interface mit Quittierungsaufforderung)");
      if (!broadcast) Parent->send->answerError("01 03 (Zentrale adressierte das Interface mit Quittierungsaufforderung)");
      break;
    case 4:                                               // Befehl ist an Zentrale geschickt
      break;
    case 5:
      Parent->Logfile->Warnings("<= Interface: Zentrale adressiert das Interface nicht mehr");
      Parent->send->pause(false);
      break;
    case 6:
      Parent->Logfile->Errors("<= Interface: Puffer-Uberlauf im Interface 23151");
      if (!broadcast) Parent->send->answerError("01 06 (Puffer-Uberlauf im Interface)");
      break;
    case 7:
      Parent->Logfile->Warnings("<= Interface: Zentrale adressiert das Interface wieder");
      Parent->send->resume();
      break;
    case 8:
      Parent->Logfile->Warnings("<= Interface: Derzeit konnen keine Befehle an die Zentrale gesendet werden");
      Parent->send->pause(!broadcast);                    // as an answer: the last message is sent again after the pause
      break;
    case 9:
      Parent->Logfile->Errors("<= Interface: Fehler in den Befehlsparametern (z.B. Lokadresse falsch)");
      if (!broadcast) Parent->send->answerRejected("01 09 (Fehler in den Befehlsparametern)", FehlerInDenBefehlsparametern);
      break;
    case 10:
      Parent->Logfile->Errors("<= Interface: Unbekannter Fehler (Zentrale lieferte nicht die erwartete Antwort)");
      if (!broadcast) Parent->send->answerError("01 0A (Zentrale lieferte nicht die erwartete Antwort)");
      break;
  }
}

// ------------------------------------------------------------------------------------------------------------------
// Ruckmeldung / Schaltinformation (V3.6: 3.1.1.5 and 3.1.8, V3.8: 2.4.5, 2.15, 2.16)
// Z21: 5.3: LAN_X_TURNOUT_INFO
// DecodeFeedback is called whenever an XpressNet Feedback message is received.
// It may carry a single feedback message (Schaltinformation) or multiple messages (Ruckmeldung).
// The message may be a point-to-point, or a broadcast message
// The input is the XpressNet message, without the Framing bytes and XOR byte
// The addresses in this routine are RS-Bus / decoder adresses.
void ReceiveClass::DecodeFeedback(quint8 length, const quint8 *message)
{
  int msgLength;                                          // The length of this XpressNet Feedback message, as indicated in the header
  int numberOfFeedbacks;                                  // A maximum of 8 Feedbacks may be contained within a single XpressNet message
  int index;                                              // Array index where the next Feedback starts
  int address;                                            // The RS-Bus / decoder address of this specific Feedback
  quint8 data;                                            // The RS-Bus / decoder data of this specific feedback
  quint8 feedbackBits;                                    // The four feedback bits that were carried in this message
  quint8 ttBits;                                          // The TT Bits inform the kind of feedback decoder that generated the message
  quint8 iBit;                                            // The I-Bit tells if the switch is still moving
  quint8 Nibble;                                          // 0 = low nibble (feedback bits 1..4). 1 = high nibble (feedbaack bits 5..8)
  // The Lenz feedback messages and the Z21 Extended Accessory Info messages have the same starting bytes
  // Therefore we first check if we have a Z21 command station (&H12).
  // If yes, we call the associated sub and exit this subroutine
  // If no, we hanlde the bytes as normal XpressNet messages
  if ((Parent->ZentraleKennung == 0x12) && (message[0] == 0x44)) {
    Parent->eventExtendedAccessoryInfo(message[1], message[2], message[3], message[4]);
    return;
  }
  // If we are here, we do not have a Z21 command station, but normal (Lenz) XpressNet command station
  // STEP 2: Determine the length of the feedback message and the number of feedbacks
  msgLength = message[0] % 16;                            // Isolated the lowest order 4 bits
  if ((msgLength + 1) != length) {                        // Extra test: if the length coded in the message matches the received size
    Parent->Logfile->Errors("Error decoding feedback message: Length doesn't match received Length");
    return;
  }
  if (msgLength == 3) {
    // STEP 3: V3.8: Schaltinformation ab W1025 / Z21: LAN_X_TURNOUT_INFO
    // The Z21 does not return the Nibble, ttBits and the iBit. Only 2 data bits are used (a single switch)
    // Two bytes for the address, plus one byte for the value
    address = (message[1] * 256) + message[2];
    // The RS-Bus data is stored in the byte immediately after
    data = message[3];
    // The "real feedback bits" are in bits 1..4 (0..3)
    feedbackBits = quint8(data % 16);
    // The nibble is coded in bit 5 (4). We can check this bit by performing an AND operation with the value 16 (0001 0000)
    Nibble = quint8((data & 16) / 16);
    // The TT bits are in coded in bits 6 and 7 (5 and 6)
    ttBits = quint8((data & 96) / 32);
    // The iBit is coded in bit 8 (7).
    iBit = quint8(data / 128);
    // Call feedback to raise a feedback event
    Parent->eventFeedback(address, Nibble, ttBits, iBit, feedbackBits);
  }
  else {
    // STEP 4: V3.6: Schaltinformation / BC Ruckmeldung
    // Analyse each feedback individually.
    numberOfFeedbacks = msgLength / 2;
    for (int i = 0; i <= (numberOfFeedbacks - 1); i++) {
      // The RS-Bus address is stored at the Array indices 1, 3, 5, 7 etc => (i * 2 + 1)
      index = 1 + (i * 2);
      address = message[index];
      // The RS-Bus data is stored in the byte immediately after
      data = message[index + 1];
      // The "real feedback bits" are in bits 1..4
      feedbackBits = quint8(data % 16);
      // The nibble is coded in bit 5. We can check this bit by performing an AND operation with the value 16 (0001 0000)
      Nibble = quint8((data & 16) / 16);
      // The TT bits are in coded in bits 6 and 7
      ttBits = quint8((data & 96) / 32);
      // The iBit is coded in bit 8.
      iBit = quint8(data / 128);
      // Call feedback to raise a feedback event
      Parent->eventFeedback(address, Nibble, ttBits, iBit, feedbackBits);
    }
  }
}
