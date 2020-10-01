//
//  DCCDecoder.h
//  CV Editor for own decoders, using Lenz LI23151 Interface
//
//  Created by Aiko Pras on 26-02-12 / 11-03-2013
//  Copyright (c) 2013 by Aiko Pras. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface DCCDecoderClass: NSObject

// Generic decoder properties
@property (assign) int decoderAddress;     // The address of the decoder we're acting on.
                                           // Will generally be equivalent to CV1 & CV9, but during
                                           // decoder initialisation may (temporary) be -1
@property (assign) int cvNumber;           // Stores the cvNumber entered at the CV Tab
@property (assign) uint8_t cvValue;        // Maintains the associated value (which can be set or get)

// Methods
- (void)initialise;
- (void)setCv:(int)number withValue:(u_int8_t)cvValue;
- (u_int8_t)getCv:(int)number;
- (NSString *)DecoderVendor;
- (NSString *)DecoderVersion;
- (NSString *)DecoderType;
- (NSString *)DecoderSubType;
- (NSString *)DecoderErrors;
- (int)getDecoderAddress;                  // Determines the decoder address using the values in CV1 and CV9
- (int)getSwitchAddress;

@end
