//
//  CvNames.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  The CV numbers, with the names of the decoder firmware (AP_DCC_Decoder_Core, AVR-Servo, Lift).
//  Same as the defines at the top of AppDelegate.m of the macOS version.
//

#ifndef CVNAMES_H
#define CVNAMES_H

namespace CV
{
  // CVs used by all decoders
  constexpr int myAddrL       = 1;
  constexpr int T_on_F1       = 3;
  constexpr int T_on_F2       = 4;
  constexpr int T_on_F3       = 5;
  constexpr int T_on_F4       = 6;
  constexpr int version       = 7;
  constexpr int VID           = 8;
  constexpr int myAddrH       = 9;
  constexpr int myRSAddr      = 10;
  constexpr int CmdStation    = 19;
  constexpr int RSRetry       = 20;
  constexpr int SkipUnEven    = 21;   // Name as in AP_DCC_Decoder_Core (before: SkipEven)
  constexpr int RSParity      = 22;
  constexpr int Search        = 23;
  constexpr int RSPulsCount   = 24;
  constexpr int Restart       = 25;
  constexpr int DccQuality    = 26;
  constexpr int DecType       = 27;
  constexpr int BiDi          = 28;
  constexpr int Config        = 29;
  constexpr int VID_2         = 30;
  constexpr int ParityErrors  = 31;   // read-only
  constexpr int PulseErrors   = 32;   // read-only
  // Switch decoder
  constexpr int SendFB        = 33;
  constexpr int AlwaysAct     = 34;
  // Lift decoder
  constexpr int StartHoming   = 33;
  constexpr int IR_Detect     = 34;
  constexpr int LCD_Display   = 35;
  constexpr int Serial_Line   = 36;
  // Servo decoders (servo_CVs.h, configure_pom.h)
  constexpr int SwitchCount             = 35;
  constexpr int SwitchType              = 36;
  constexpr int CV_POM_CONFIG_MODE      = 62;   // RAM only: what should be configured (NoMode, MinMode, MaxMode, SpeedMode)
  constexpr int CV_POM_CONFIG_SERVO     = 63;   // RAM only: which servo should be configured (0 = none, 1..CV64)
  constexpr int EEPROM_MAX_SERVOS_INDEX = 64;   // Maximum number of servos of the board (read only)
  constexpr int START_INDEX_SERVO_CVS   = 65;
  constexpr int NUMBER_OF_SERVO_CVS     = 18;
}

// Offsets of the CVs of each servo. CV number of a servo CV = 65 + 18 x (servo - 1) + offset
namespace ServoCv {
  constexpr int MinLow            = 0;
  constexpr int MinHigh           = 1;
  constexpr int MaxLow            = 2;
  constexpr int MaxHigh           = 3;
  constexpr int CurveA            = 4;
  constexpr int CurveB            = 5;
  constexpr int Speed             = 6;
  constexpr int InvertServoDir    = 7;
  constexpr int InvertRelais      = 8;    // Only used by the servo-2 board (relays)
  constexpr int ServoType         = 9;
  constexpr int PulseStartUpValue = 10;
  constexpr int PowerStartUpDelay = 11;
  constexpr int IdlePulseDefault  = 12;
  constexpr int PulseOnBefore     = 13;
  constexpr int PulseOffAfter     = 14;
  constexpr int PowerWhenIdle     = 15;
  constexpr int PowerOnBefore     = 16;
  constexpr int PowerOffAfter     = 17;
}

// Values of SwitchType (CV36 of the servo decoders)
enum SwitchTypes {NORMAL = 0, DKW = 1, HOSENTRAGER = 2};

// Values of CV_POM_CONFIG_MODE (CV62 of the servo decoders)
enum LiveModes {NoMode = 0, MinMode = 1, MaxMode = 2, SpeedMode = 3};

#endif // CVNAMES_H
