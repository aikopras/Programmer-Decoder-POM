//
//  DCCDecoder.m
//  CV Editor for own decoders, using Lenz LI23151 Interface
//
//  Created by Aiko Pras on 26-02-12 / 11-03-2013
//  Copyright (c) 2013 by Aiko Pras. All rights reserved.
//

#import "DCCDecoder.h"

@implementation DCCDecoderClass

@synthesize decoderAddress;
@synthesize cvNumber;
@synthesize cvValue;

// ************************************************************************************************************
// ******************************************** DEFINE CV MAPPINGS ********************************************
// ************************************************************************************************************
#define myAddrL        1
#define version        7
#define VID            8
#define myAddrH        9
#define myRSAddr      10
#define DccQuality    26
#define DecType       27
#define VID_2         30


// ************************************************************************************************************
// ************************************** DECLARATION AND USE OF THE CVSTORE **********************************
// ************************************************************************************************************
#define MAX_CVS 256                    // Number of CVs we support. Note that we map 257->1, 513->1 etc.
uint8_t cvStore[MAX_CVS + 1];          // Array is indexed by CV number, and hold its current value


- (void)setCv:(int)number withValue:(u_int8_t)value{
  cvStore[number] = value;
}

- (u_int8_t)getCv:(int)number{
  return cvStore[number];
}

- (void) initialise {
  for (int i=0; i <= (MAX_CVS) ; i++) {cvStore[i] = 0;};
}


// ************************************************************************************************************
// ************************************** METHODS SUPPORTED BY ALL DECODERS  **********************************
// ************************************************************************************************************
- (NSString *)DecoderVendor{
  NSString *string = @"Not supported";
  // Decoder Vendor: Use VID and VID_2
  if (cvStore[VID] == 0x0D) {
    string = @"Do It Yourself";
    if (cvStore[VID_2] == 0x0D) string = @"Aiko Pras"; }
  return string;
}

- (NSString *)DecoderVersion{
  return [NSString stringWithFormat:@"%d", cvStore[version]];;
}


- (NSString *)DecoderType{
  NSString *string = @"unknown";
  if      (cvStore[DecType] == 16)  {string = @"Swith decoder";}
  else if (cvStore[DecType] == 17)  {string = @"Swith decoder";}
  else if (cvStore[DecType] == 20)  {string = @"Servo Decoder";}
  else if (cvStore[DecType] == 32)  {string = @"Relays decoder";}
  else if (cvStore[DecType] == 33)  {string = @"Relays decoder";}
  else if (cvStore[DecType] == 48)  {string = @"Track Occupancy decoder";}
  else if (cvStore[DecType] == 49)  {string = @"Track Occupancy decoder";}
  else if (cvStore[DecType] == 50)  {string = @"Track Occupancy decoder";}
  else if (cvStore[DecType] == 52)  {string = @"Track Occupancy decoder";}
  else if (cvStore[DecType] == 64)  {string = @"Function Decoder";}
  else if (cvStore[DecType] == 128) {string = @"Safety decoder";}
  return string;
}

- (NSString *)DecoderSubType{
  NSString *string = @"";
  if      (cvStore[DecType] == 16)  {string = @"";}
  else if (cvStore[DecType] == 17)  {string = @"with Emergency board";}
  else if (cvStore[DecType] == 20)  {string = @"";}
  else if (cvStore[DecType] == 32)  {string = @"for 4 relais";}
  else if (cvStore[DecType] == 33)  {string = @"for 16 relais";}
  else if (cvStore[DecType] == 48)  {string = @"";}
  else if (cvStore[DecType] == 49)  {string = @"with reverser board";}
  else if (cvStore[DecType] == 50)  {string = @"with relays board";}
  else if (cvStore[DecType] == 52)  {string = @"with speed measurement";}
  else if (cvStore[DecType] == 64)  {string = @"";}
  else if (cvStore[DecType] == 128) {string = @"with Watchdog";}
  return string;
}

- (NSString *)DecoderErrors{
  return [NSString stringWithFormat:@"%d", cvStore[DccQuality]];;
}

- (int)getDecoderAddress{
  // Create the decoder address from the values of CV1 and CV9
  // Note that the decoder software got changed in 2020 (starting from version 16)
  // For the decoders with version <  16: decoderAddress = CV9 << 6 + CV1
  // For the decoders with version >= 16: decoderAddress = CV9 << 6 + CV1 - 1
  int cv1 = [self getCv:myAddrL];
  int cv9 = [self getCv:myAddrH];
  int myDecAddr = ((cv9 & 0x7F) << 6) | (cv1); // 3 bits from cv9 (the high bits) plus 6 bits from cv1
  if ([self getCv:version] >= 16) myDecAddr = myDecAddr - 1;
  return (myDecAddr);
}

- (int)getSwitchAddress{
  int myDecAddr = [self getDecoderAddress];
  int switchAddress = myDecAddr * 4 + 1;  // Compensate since we store blocks of four and 0..1023 instead of 1.1024
  return switchAddress;
}

@end
