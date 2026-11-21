//
//  DccDecoder.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of DCCDecoder.m of the macOS version: holds the CV values of the selected decoder,
//  and derives the vendor, version, type, subtype, number of errors and the addresses from these CVs.
//

#include "DccDecoder.h"
#include "CvNames.h"

DccDecoder::DccDecoder()
{
  decoderAddress = 0;
  initialise();
}

void DccDecoder::initialise()
{
  for (int i = 0; i <= MAX_CVS; i++) cvStore[i] = 0;
}

// We map CV513 -> CV1 etc. (as the macOS version)
static int index(int number)
{
  if (number < 0) return 0;
  if (number > DccDecoder::MAX_CVS) return number % DccDecoder::MAX_CVS;
  return number;
}

void DccDecoder::setCv(int number, uint8_t value)
{
  cvStore[index(number)] = value;
}

uint8_t DccDecoder::getCv(int number) const
{
  return cvStore[index(number)];
}


// ************************************************************************************************************
// ************************************** METHODS SUPPORTED BY ALL DECODERS  **********************************
// ************************************************************************************************************
QString DccDecoder::decoderVendor() const
{
  QString string = "Not supported";
  // Decoder Vendor: Use VID and VID_2
  if (cvStore[CV::VID] == 0x0D) {
    string = "Do It Yourself";
    if (cvStore[CV::VID_2] == 0x0D) string = "Aiko Pras";
  }
  return string;
}

QString DccDecoder::decoderVersion() const
{
  return QString::number(cvStore[CV::version]);
}

QString DccDecoder::decoderType() const
{
  switch (cvStore[CV::DecType]) {
    case 16:  return "Switch decoder";
    case 17:  return "Switch decoder";
    case 20:  return "Servo-2 Decoder";
    case 21:  return "Servo-3 Decoder";
    case 22:  return "Servo-6 Decoder";
    case 24:  return "Lift Decoder";
    case 32:  return "Relays decoder";
    case 33:  return "Relays decoder";
    case 48:  return "Track Occupancy decoder";
    case 49:  return "Track Occupancy decoder";
    case 50:  return "Track Occupancy decoder";
    case 52:  return "Track Occupancy decoder";
    case 64:  return "Function Decoder";
    case 128: return "Safety decoder";
    default:  return "unknown";
  }
}

QString DccDecoder::decoderSubType() const
{
  switch (cvStore[CV::DecType]) {
    case 17:  return "with Emergency board";
    case 32:  return "for 4 relais";
    case 33:  return "for 16 relais";
    case 49:  return "with reverser board";
    case 50:  return "with relays board";
    case 52:  return "with speed measurement";
    case 128: return "with Watchdog";
    default:  return "";
  }
}

QString DccDecoder::decoderErrors() const
{
  return QString::number(cvStore[CV::DccQuality]);
}

int DccDecoder::getDecoderAddress() const
{
  // Create the decoder address from the values of CV1 and CV9
  // Note that the decoder software got changed in 2020 (starting from version 16)
  // For the decoders with version <  16: decoderAddress = CV9 << 6 + CV1
  // For the decoders with version >= 16: decoderAddress = CV9 << 6 + CV1 - 1
  int cv1 = getCv(CV::myAddrL);
  int cv9 = getCv(CV::myAddrH);
  int myDecAddr = ((cv9 & 0x7F) << 6) | cv1;   // 3 bits from cv9 (the high bits) plus 6 bits from cv1
  if (getCv(CV::version) >= 16) myDecAddr = myDecAddr - 1;
  return myDecAddr;
}

int DccDecoder::getSwitchAddress() const
{
  // Compensate since we store blocks of four and 0..1023 instead of 1..1024
  return getDecoderAddress() * 4 + 1;
}
