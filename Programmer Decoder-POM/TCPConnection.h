//
//  TCPConnection.h
//  CV Editor for general decoders based on OPENDECODER22
//
//  Created by Aiko Pras on 26-02-12 / 11-03-2013
//  Copyright (c) 2013 by Aiko Pras. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DCCDecoder.h"
#import "AppDelegate.h"

@interface TCPConnectionsClass : NSObject <NSStreamDelegate>

// The following methods should be used.
// Note that flow-control between computer and Lenz system / decoder is in general necessary
// Therefore first queue all CVs of interest, and subsequently initiate sending from queue
- (void)openTcp;
- (void)closeTcp;
- (void)queuePomWritePacketForCV:(int)cvNumber;
- (void)queuePomVerifyPacketForCV:(int)cvNumber;
- (void)sendNextPomWritePacketFromQueue;
- (void)sendNextPomVerifyPacketFromQueue;


// Some additional methods are defined that do not exercise flow control.
// Inproper use may have unpredictable effects. Use flow controlled methods from above instead
- (void)sendPomWritePacketForCV:(int)cvNumber withValue:(u_int8_t)cvValue;
- (void)sendPomVerifyPacketForCV:(int)cvNumber;


// The TCP object may be in any of the following operational states
typedef enum
{ INACTIVE = 0,
  POM_WRITE = 1,
  POM_VERIFY = 2,
  STATUS_LENZ_1 = 3,
  STATUS_LENZ_2 = 4,
} operationalState_t;
@property (assign) operationalState_t operationalState;



// The following properties are for internal use, and should not be used outside
@property (assign) AppDelegate        *topObject;
@property (strong) NSInputStream      *iStreamPoM;
@property (strong) NSOutputStream     *oStreamPoM;
@property (strong) NSInputStream      *iStreamRS;
@property (strong) NSOutputStream     *oStreamRS;
@property (assign) NSString           *ipAddressForSending;
@property (assign) NSString           *ipAddressForReceiving;
@property (assign) Boolean            ipAddressForReceivingIsActive;


@end
