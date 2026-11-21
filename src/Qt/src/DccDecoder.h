//
//  DccDecoder.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  See DccDecoder.cpp
//

#ifndef DCCDECODER_H
#define DCCDECODER_H

#include <QString>
#include <cstdint>

class DccDecoder
{
public:
  static constexpr int MAX_CVS = 512;     // Number of CVs we support. Note that we map 513->1 etc.

  DccDecoder();
  void initialise();
  void setCv(int number, uint8_t value);
  uint8_t getCv(int number) const;

  // The decoder address (0..255) used for PoM messages. -1 is used for non-initialised decoders
  int decoderAddress;

  QString decoderVendor() const;
  QString decoderVersion() const;
  QString decoderType() const;
  QString decoderSubType() const;
  QString decoderErrors() const;
  int getDecoderAddress() const;          // Calculated from CV1 and CV9
  int getSwitchAddress() const;           // Calculated from CV1 and CV9

private:
  uint8_t cvStore[MAX_CVS + 1];           // Indexed by CV number, and holds its current value
};

#endif // DCCDECODER_H
