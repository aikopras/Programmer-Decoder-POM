//
//  TCPConnection.m
//  CV Editor for own decoders, using Lenz LI23151 Interface
//
//  Created by Aiko Pras 2012 & 2013.
//
// This file contains the code to communicate via TCP to Lenz LI23151 Interface(s)
// This file does not contain (switch or feedback) decoder specific code.
// The code allows sending CV PoM write messages as well as CV PoM verify messages
// My own decoders will react on reception of a CV PoM verify message by sending a RS-bus feedback message
// This code will also receive and interpret such RS-bus feedback messages
//  
// It is possible to use a single Lenz LI23151 for both sending PoM messages and receiving RS-bus feedback messages
// However, it is also possible to use two Lenz LI23151 interfaces: one for sending and one for receiving
//
// We may be in one of the following states: INACTIVE, POM_WRITE, POM_VERIFY, STATUS_LENZ_1 or STATUS_LENZ_2.
// POM_WRITE:
// Sending Pom messages is relatively straigtforward, which means that error handling is simple.
// There is a timer (PomTimeOut) to determine if the interface is still responding.
// If not: release the TCP connections
// POM_VERIFY:
// If a CV PoM verify message is send, we expect a RS-bus feedback message in return.
// In that case several errors are possible:
// 1) since RS-bus messages can carry only 4 bit of user data, a CV value is send using two consequetive RS-bus messages
// One of these RS-bus messages may get lost, however. We react on that by also ignoring the other 4 bits in the other message.
// As a consequence of ignoring the other 4 bit message, no feedback value will be created and a RsTimeOut will occur.
// 2) A RsTimeOut of the (reassembled 8 bit) response message may also occur in the following situations:
// 2a) the Lenz interface used for sending PoM messages does not react. In this case further processing is useless.
// 2b) the Lenz interface used for receiving feedback messages does not react. In this case we can continue sending PoM set messages.
// 2c) no decoder is listening to the address being specified. We detect this case by sending verify requests to well supported CVs (CV1 & CV8)
// 2d) the decoder does not react on the CV being requested. If the decoder reacts to the well supported CVs mentioned above, this must be the case.
// 2e) some temporary problem (like loss of a RS-Bus message). We just try to verify the CV a second time.


#import "TCPConnection.h"
#import "DCCDecoder.h"
#import "AppDelegate.h"


@implementation TCPConnectionsClass

@synthesize topObject                     = _topObject;
@synthesize iStreamPoM                    = _iStreamPoM;
@synthesize oStreamPoM                    = _oStreamPoM;
@synthesize iStreamRS                     = _iStreamRS;
@synthesize oStreamRS                     = _oStreamRS;
@synthesize ipAddressForSending           = _ipAddressForSending;
@synthesize ipAddressForReceiving         = _ipAddressForReceiving;
@synthesize ipAddressForReceivingIsActive = _ipAddressForReceivingIsActive;
@synthesize operationalState              = _operationalState;

// ***************************************************************************************************************************************************
// ******************************************************* Local C type definitions and declarations *************************************************
// ***************************************************************************************************************************************************
#define POM_OFFSET 7000                   // First PoM address starts from this address
#define RS_BUS_TIMEOUT 0.25               // timeout, in seconds, to receive a RS-Bus feedback response to a previous PoM verify message
#define POM_TIMEOUT 0.1                   // timeout, to receive a "Request Forwarded to Master Station" response to a previous PoM write message
#define POM_WRITE_DELAY 0.05              // Extra delay we introduce between PoM write requests
                                          //
                                          // Declarations below should preferable not be changed
#define MAX_CVS 256                       // Number of CVs we support. Note that we map 257->1, 513->1 etc.
#define MAX_INSTREAM 1                    // size TCP input stream for the LENZ interface(s)
#define MAX_INBUFFER 32                   // max. TCP input buffer size for the LENZ interface(s)
                                          // We keep two sets of buffer variables: one for the interface over which PoM messages are send,
                                          // and one for the interface from which we receive Feedback messages.
                                          // Note that both connections may go to the same physical LENZ interface
                                          // Interface for sending PoM messages first
uint8_t pomInStream[MAX_INSTREAM];        // TCP input stream buffer for the LENZ interface sending PoM  messages
uint8_t pomInBuffer[MAX_INBUFFER];        // Frame buffer holding (parts of) the message received thusfar from the PoM interface
int pomInputBufferSize = 0;               // Size (until now) of the buffer holding (parts of) the PoM interface message
int totalPomBytes = 0;                    // Total number of bytes we have received thusfar via the interface PoM messages
int pomSynchronized = 0;                  // Indicates if we know the beginning of the received frame from the PoM interface
                                          // RS-bus feedback interface next
uint8_t rsBusInStream[MAX_INSTREAM];      // TCP input stream buffer for the LENZ interface receiving RS-bus feedback messages
uint8_t rsBusInBuffer[MAX_INBUFFER];      // Frame buffer holding (parts of) the feedback message received thusfar
int rsBusInputBufferSize = 0;             // Size (until now) of the buffer holding (parts of) the feedback message
int totalRsBusBytes = 0;                  // Total number of bytes we have received thusfar via the interface for feedback messages
int rsBusSynchronized = 0;                // Indicates if we know the beginning of the RS-bus feedback frame
uint8_t nibble1Value = 0;                 // Temporarily stores the first received nibble to combine later with nibble2
uint8_t waiting_for_second_nibble = 0;    // Flag to check if we received already the first nibble
int previousVerifyCv = 0;                 // We store the CV of the last PoM verify request, for status info and to allow later retransmission
int numberOfVerifyCvs = 0;                // Counts the number of PoM verify request performed as part of this query
int numberOfWriteCvs = 0;                 // Counts the number of PoM write request performed as part of this query
int numberOfVerifyReceived = 0;           // Counts the number of PoM verify responses received as part of this query
                                          //
                                          // We maintain a queue of PoM Write and PoM Verify request, to exercise flow control
struct cv_write_t {                       // For each CV we'll write we need to know the CV number (= array index), value plus a flag whether we should write
  uint8_t cvValue;                        // The value this CV should get
  uint8_t cvIsQueued; };                  // 0: no write action needed. 1: CV value should be written
struct cv_write_t cvs_write[MAX_CVS + 1]; // Array is indexed by CV number. Note that position 0 in the Array is not used, since CV numbers start with 1
uint8_t cvs_verify[MAX_CVS + 1];          // Array is indexed by CV number. An array value > 0 indicates we need to retrieve this CV



// Error states possible within the POM_VERIFY operational status (see associated .h file for definition) 
typedef enum
{
  NoError = 0,
  CheckingSendingLenz = 1,
  CheckingReceivingLenz = 2,
  CheckingCv1 = 3,
  CheckingCv8 = 4,
  RetryCv = 5,
  NoFeedbacks = 6,
  NoCommunicationPossible = 7
} error_state_t;
error_state_t ErrorStatePomVerify = NoError;


// Some types and variables to store status info regarding the Lenz Interface
struct lenzStatus_t {
  uint8_t softwareVersionMain;
  uint8_t softwareVersionMinor;
  uint8_t softwareCode;
  uint8_t xpressnetAddress;
  uint8_t xpressnetVersionMain;
  uint8_t xpressnetVersionMinor;
  uint8_t tcpConnectionsFree;
};

struct lenzStatus_t lenzStatusPom;
struct lenzStatus_t lenzStatusRS;


// ****************************************************************************************************************************************************
// ********************************************************** MAIN METHODS CALLED FROM OUTSIDE ********************************************************
// ******************************************************************* CONNECT METHODS ****************************************************************
// ****************************************************************************************************************************************************
- (void)openTcp{
  // Make sure we can access the properties and methods of the APPDelegate Object
  _topObject = ((AppDelegate *) [[NSApplication sharedApplication] delegate]);
  [_topObject progressIndicator: 1];
  [self openTcpPoM];
  [self openTcpRs];
  _operationalState = INACTIVE;
  [self clear_cvs_write];   // Empty queue for PoM write messages
  [self clear_cvs_verify];  // Empty queue for PoM verify messages
  // Query the status of both interfaces, but wait a while till things are running
  [self performSelector:@selector(queryStatusLenz1) withObject:nil afterDelay:1];
  [self performSelector:@selector(queryStatusLenz2) withObject:nil afterDelay:1.2];
}

- (void)closeTcp{
  [self closeTcpPoM];
  [self closeTcpRs];
  _operationalState = INACTIVE;
  ErrorStatePomVerify = NoError;
  // Update the status line manually, now that the runloop will not call this anymore
  [_topObject showGeneralStatus: @"TCP connection to Lenz interface(s) has been closed"];
  [_topObject progressIndicator: 0];
};


// The four open and close methods below are local
- (void)openTcpPoM{
  // Note: this code is intended for MAC OSx 10.6 or higher. ARC is turned off.
  _ipAddressForSending = [[NSUserDefaults standardUserDefaults] objectForKey:@"defaultIpAddressForSending"];

  // Changed 2020, since getStreamsToHost was depricated and should be replaced by getStreamsToHostWithName
  // The following two lines were removed, the third line was added
  // NSHost *host = [NSHost hostWithName:_ipAddressForSending];
  // [NSStream getStreamsToHost:host port:5550   inputStream:&(_iStreamPoM) outputStream:&(_oStreamPoM)];
  [NSStream getStreamsToHostWithName:_ipAddressForSending port:5550   inputStream:&(_iStreamPoM) outputStream:&(_oStreamPoM)];

    // NSLog(@"TCP connect attempt PoM: %@", host.address);
  if (_iStreamPoM == nil || _oStreamPoM == nil) {[_topObject showGeneralStatus: @"Error opening TCP connection for PoM messages"]; return;}       
  [_iStreamPoM retain];
  [_oStreamPoM retain];
  // set the delegate
  [_iStreamPoM setDelegate:self];
  [_oStreamPoM setDelegate:self];
  // put both streams in the run-loop
  [_iStreamPoM scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
  [_oStreamPoM scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
  // open both streams
  [_iStreamPoM open];
  [_oStreamPoM open];
}

- (void)openTcpRs{
  // There is a checkbox in the preferences window to allow "activation" of the second (feedback) interface  
  _ipAddressForReceivingIsActive = [[NSUserDefaults standardUserDefaults] boolForKey:@"defaultIpAddressForReceivingIsActive"];
  if (_ipAddressForReceivingIsActive == 0) return;
  _ipAddressForReceiving = [[NSUserDefaults standardUserDefaults] objectForKey:@"defaultIpAddressForReceiving"];
  
  // Changed 2020, since getStreamsToHost was depricated and should be replaced by getStreamsToHostWithName
  // The following two lines were removed, the third line was added
  // NSHost *host = [NSHost hostWithName:_ipAddressForReceiving];
  // [NSStream getStreamsToHost:host port:5550   inputStream:&(_iStreamRS) outputStream:&(_oStreamRS)];
  [NSStream getStreamsToHostWithName:_ipAddressForReceiving port:5550   inputStream:&(_iStreamRS) outputStream:&(_oStreamRS)];
  
  // NSLog(@"TCP connect attempt RS: %@", host.address);
  if (_iStreamRS == nil || _oStreamRS == nil) {[_topObject showGeneralStatus: @"Error opening TCP connection for RS-bus feedbacks"]; return;}
  [_iStreamRS retain];
  [_oStreamRS retain];
  // set the delegate
  [_iStreamRS setDelegate:self];
  [_oStreamRS setDelegate:self];
  // put both streams in the run-loop
  [_iStreamRS scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
  [_oStreamRS scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
  // Since we can live without the RSbus connection (so we do not receive feedbacks)
  // open both streams
  [_iStreamRS open];
  [_oStreamRS open];
}

- (void)closeTcpPoM{
  if (_iStreamPoM != nil) {
    // Close and remove the TCP input stream
    [_iStreamPoM close]; // Note that this call will NOT close the TCP connection (TCP FIN)
    [_iStreamPoM removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [_iStreamPoM release];
    _iStreamPoM = nil; // stream is instance variable, so reinit it
    // Close and remove the TCP output stream
    [_oStreamPoM close]; // Only this call will close the TCP connection (TCP FIN)
    [_oStreamPoM removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [_oStreamPoM release];
    _oStreamPoM = nil; // stream is instance variable, so reinit it
  }
}

- (void)closeTcpRs{
  if (_iStreamRS != nil) {
    // Close and remove the TCP input stream
    [_iStreamRS close]; // Note that this call will NOT close the TCP connection (TCP FIN)
    [_iStreamRS removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [_iStreamRS release];
    _iStreamRS = nil; // stream is instance variable, so reinit it
    // Close and remove the TCP output stream
    [_oStreamRS close]; // Only this call will close the TCP connection (TCP FIN)
    [_oStreamRS removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [_oStreamRS release];
    _oStreamRS = nil; // stream is instance variable, so reinit it
  }
}


// ****************************************************************************************************************************************************
// ********************************************************** MAIN METHODS CALLED FROM OUTSIDE ********************************************************
// ***************************************************** REQUESTS MUST BE QUEUED BEFORE TRANSMISSION **************************************************
// ****************************************************************************************************************************************************
- (void)queuePomWritePacketForCV:(int)cvNumber {
  // We map CV257->CV1, CV513_CV1 etc. Thus we operate modulo MAX_CVS
  int adjustedCvNumber = cvNumber % MAX_CVS;
  cvs_write[adjustedCvNumber].cvValue = [_topObject.dccDecoderObject getCv:adjustedCvNumber];
  cvs_write[adjustedCvNumber].cvIsQueued = 1;
}


- (void)queuePomVerifyPacketForCV:(int)cvNumber {
  // Do not queue any verify packets if the TCP connection to receive feedbacks is not open
  if (_iStreamRS == nil) return;
  // We map CV257->CV1, CV513_CV1 etc. Thus we operate modulo MAX_CVS
  int adjustedCvNumber = cvNumber % MAX_CVS;
  cvs_verify[adjustedCvNumber] = 1;
}


- (void)sendNextPomWritePacketFromQueue {
  // Sending will start if we are in the inactive state.
  // Requests being queued before will be deleted if don't start from the INACTIVE state
  if (_operationalState == INACTIVE) {
    // First packet: Clear status lines and change STATE
    [_topObject showSendStatus: @""];
    [_topObject showReceiveStatus: @""];
    _operationalState = POM_WRITE;
  }
  if (_operationalState != POM_WRITE) {[self clear_cvs_write]; return;}
  [_topObject progressIndicator: 1];
  int nextCv = [self getNextWriteCvFromQueue];
  if (nextCv > 0){
    numberOfWriteCvs ++;
    [self sendPomWritePacketForCV:nextCv withValue:cvs_write[nextCv].cvValue];
    // NSLog(@"sendNextPomWritePacketFromQueue for CV:%d with value:%d", nextCv, cvs_write[nextCv].cvValue);
  }
  else { // No more write messages to send
    // Write the number of POM packets written to the send status line
    NSString *message = @"Number of transmitted POM write messages: ";
    message = [message stringByAppendingString:[NSString stringWithFormat:@"%d",numberOfWriteCvs]];
    [_topObject showSendStatus: message];
    // Inform the AppDelegate
    [_topObject sendNextPomWritePacketFromQueueCompleted];
    // Clear everything
    numberOfWriteCvs = 0;
    _operationalState = INACTIVE;
    [self clear_cvs_write];
    [_topObject progressIndicator: 0];
  }
}


- (void)sendNextPomVerifyPacketFromQueue {
  if (_operationalState == INACTIVE) {
    // First packet: Clear status lines and change STATE
    [_topObject showSendStatus: @""];
    [_topObject showReceiveStatus: @""];
    _operationalState = POM_VERIFY;
  }
  if (_operationalState != POM_VERIFY) return;
  [_topObject progressIndicator: 1];
  if (waiting_for_second_nibble == 0) {
    int nextCv = [self getNextVerifyCvFromQueue];
    if (nextCv > 0){
      // OK, there is another CV to be verified. Lets do that
      previousVerifyCv = nextCv; // store for status messages and possible later retransmission
      waiting_for_second_nibble = 1;
      numberOfVerifyCvs ++;
      [self sendPomVerifyPacketForCV:nextCv];
      // NSLog(@"sendNextPomVerifyPacketFromQueue for CV:%d", nextCv);
    }
    else { // No more verify messages to send
      // Clear the general status line
      [_topObject showGeneralStatus: @""];
      // Write the number of POM packets verified to the send status line
      NSString *message = @"Number of transmitted POM verify messages: ";
      message = [message stringByAppendingString:[NSString stringWithFormat:@"%d",numberOfVerifyCvs]];
      [_topObject showSendStatus: message];
      // Write the number of received POM verify response messages to the send status line
      // The counter "numberOfVerifyReceived" is incremented in DetermineFeedbackValueFor...
      message = @"Number of received POM verify response messages: ";
      message = [message stringByAppendingString:[NSString stringWithFormat:@"%d",numberOfVerifyReceived]];
      [_topObject showReceiveStatus: message];
      // Inform the AppDelegate
      [_topObject sendNextPomVerifyPacketFromQueueCompleted];
      // Clear everything
      numberOfVerifyCvs = 0;
      numberOfVerifyReceived = 0;
      _operationalState = INACTIVE;
      [self clear_cvs_verify];
      [_topObject progressIndicator: 0];
    }
  }
}


// Methods below are local support routines
- (void)clear_cvs_write  {for(int i=0;i<=MAX_CVS;i++) {cvs_write[i].cvValue = 0; cvs_write[i].cvIsQueued = 0;}}
- (void)clear_cvs_verify {for(int i=0;i<=MAX_CVS;i++) {cvs_verify[i] = 0;}}


- (int)getNextWriteCvFromQueue {
  for(int i=1;i<=MAX_CVS;i++) {
    if (cvs_write[i].cvIsQueued != 0) {cvs_write[i].cvIsQueued = 0; return i;}}
  return 0;
}

- (int)getNextVerifyCvFromQueue {
  for(int i=1;i<=MAX_CVS;i++) {
    if (cvs_verify[i] != 0) {cvs_verify[i] = 0; return i;}}
  return 0;
}

- (void)retryPreviousdPomVerifyPacket {
  waiting_for_second_nibble = 1;
  [self sendPomVerifyPacketForCV:previousVerifyCv];
}


// ****************************************************************************************************************************************************
// ********************************************************** RUN LOOP CHECKING TCP ACTIVITY **********************************************************
// ****************************************************************************************************************************************************
- (void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)eventCode
{ // this routine receives individual bytes from the input stream, and creates a frame
  if (stream == _iStreamPoM) {
    switch(eventCode) {
      case NSStreamEventNone:               { break;}
      case NSStreamEventHasSpaceAvailable:  { break;}
      case NSStreamEventErrorOccurred:      { [self closeTcp]; break;}
      case NSStreamEventEndEncountered:     { break;}
      case NSStreamEventOpenCompleted:      { [_topObject showGeneralStatus: @"Connected to Lenz Interface for Sending PoM messages"];;break;}
      case NSStreamEventHasBytesAvailable:  { [self readByteFromPomStream:stream]; break;}
    }
  }
  if (stream == _oStreamPoM) {
    switch(eventCode) {
      case NSStreamEventNone:               { break;}
      case NSStreamEventHasSpaceAvailable:  { break;}
      case NSStreamEventErrorOccurred:      { [self closeTcp]; break;}
      case NSStreamEventEndEncountered:     { break;}
      case NSStreamEventOpenCompleted:      { [_topObject progressIndicator: 0]; break;}
      case NSStreamEventHasBytesAvailable:  { break;}
    }
  }
  if (stream == _iStreamRS) {
    switch(eventCode) {
      case NSStreamEventNone:               { break;}
      case NSStreamEventHasSpaceAvailable:  { break;}
      case NSStreamEventErrorOccurred:      { [self closeTcp]; break;}
      case NSStreamEventEndEncountered:     { break;}
      case NSStreamEventOpenCompleted:      { [_topObject showGeneralStatus: @"Connected to Lenz Interface(s)"];;break;}
      case NSStreamEventHasBytesAvailable:  { [self readByteFromRsBus:stream]; break;}
    }
  }
  if (stream == _oStreamRS) {
    switch(eventCode) {
      case NSStreamEventNone:               { break;}
      case NSStreamEventHasSpaceAvailable:  { break;}
      case NSStreamEventErrorOccurred:      { [self closeTcp]; break;}
      case NSStreamEventEndEncountered:     { break;}
      case NSStreamEventOpenCompleted:      { break;}
      case NSStreamEventHasBytesAvailable:  { break;}
    }
  }
}


// ****************************************************************************************************************************************************
// ****************************************************************** SEND PoM METHODS ****************************************************************
// ****************************************************************************************************************************************************
- (void)sendPomWritePacketForCV:(int)cvNumber withValue:(u_int8_t)cvValue {
  unsigned int i;   // Counter for FOR loop
  // Get the decoderAddress property from the dccDecoder Object
  int adr = [_topObject.dccDecoderObject decoderAddress];
  // Make some checks if values are OK
  // Decoder addresses range from 0..255. Note that we accept Address = -1, since that address may be used for initialisation
  if (adr < -1)        {[_topObject showGeneralStatus: @"Decoder address should be between 0-255"]; return; }
  if (adr > 255)       {[_topObject showGeneralStatus: @"Decoder address should be between 0-255"]; return; }
  if (cvNumber <= 0)   {[_topObject showGeneralStatus: @"CV should be between 1-1024"]; return; }
  if (cvNumber > 1024) {[_topObject showGeneralStatus: @"CV too high"]; return; }
  // We have to add an offset of 7000 to the decoder address to find the loco address
  // Loco address range: 7000-1 for initialization till 7255
  adr = adr + POM_OFFSET;
  int aH = ((adr&0xFF00) / 256) + 0xC0;
  int aL = (adr&0x00FF);
  // CV values on LH-100 range between 1 and 1024; on the rails between 0 and 1023
  cvNumber = cvNumber - 1;
  // calculate the cvH and cvL bytes from the cvNumber
  int cvH = ((cvNumber&0x0300) / 256);
  int cvL = cvNumber - (cvH * 256);
  // create an integer value for cvValue
  int cvIntValue = cvValue;
  // Assignment of the packet buffer variable, and enter the packet data
  NSUInteger length = 10;
  uint8_t buffer[length];
  buffer[0] = 0xFF;        // Frame 1
  buffer[1] = 0xFE;        // Frame 2
  buffer[2] = 0xE6;        // Header byte - Programming on Main - Byte schreiben
  buffer[3] = 0x30;        // Kennung
  buffer[4] = aH;          // Daten 1 - Address High
  buffer[5] = aL;          // Daten 2 - Address Low
  buffer[6] = cvH + 0xEC;  // Daten 3 - CV Address - 2 most significant bits
  buffer[7] = cvL;         // Daten 4 - CV Address - 8 remaining bits
  buffer[8] = cvIntValue;  // Daten 5 - CV value
  buffer[9] = 0x00;        // XOR value
  length = 10;
  // create the checksum (XOR) byte)
  unsigned char myxor = 0;
  for (i=2; i < (length-1) ; i++) { myxor = myxor ^ buffer[i];}
  buffer[length -1] = myxor;      // XOR value
  // send the actual data over the TCP connection for outgoing PoM messages
  [_oStreamPoM write:(const uint8_t *)buffer maxLength:length];
  // to determine if we receive a reply, start the time out timer
  [self startPomTimeOut];
  // Write the value of the POM packet to the send status line
  NSString *message = @"POM write message send: CV=";
  message = [message stringByAppendingString:[NSString stringWithFormat:@"%d",cvNumber + 1]];
  message = [message stringByAppendingString:[NSString stringWithFormat:@" Value="]];
  message = [message stringByAppendingString:[NSString stringWithFormat:@"%d",cvValue]];
  // message = [self packetToString:buffer withLength:length];
  [_topObject showSendStatus: message];
}


- (void)sendPomVerifyPacketForCV:(int)cvNumber {
  unsigned int i;
  // Get the decoderAddress property from the dccDecoder Object
  int adr = [_topObject.dccDecoderObject decoderAddress];
  // Make some checks if values are OK.
  // Decoder addresses range from 0..255
  // Note that we accept Address = -1 for initialisation at the programm start
  if (adr < -1)         {[_topObject showGeneralStatus: @"Decoder address should be between 0-255"]; return; }
  if (adr > 255)       {[_topObject showGeneralStatus: @"Decoder address should be between 0-255"]; return; }
  if (cvNumber == 0)   {[_topObject showGeneralStatus: @"CV should be between 1-1024"]; return; }
  if (cvNumber > 1024) {[_topObject showGeneralStatus: @"CV too high"]; return; }
  // We have to add an offset of 7000 to the decoder address to find the loco address
  // Loco address range: 7000 till 7255
  adr = adr + POM_OFFSET;
  // NSLog(@"Addr:%i",adr);
  int aH = ((adr&0xFF00) / 256) + 0xC0;
  int aL = (adr&0x00FF);
  // CV values on LH-100 range between 1 and 1024; on the rails between 0 and 1023
  cvNumber = cvNumber - 1;
  // calculate the cvH and cvL bytes from the cvNumber
  int cvH = ((cvNumber&0x0300) / 256);
  int cvL = cvNumber - (cvH * 256);
  // create an integer value for cvValue
  int cvIntValue = 0;
  // do not send DCC POM packets if RS-bus address address = 0 (mind the offset)
  NSUInteger length = 10;
  uint8_t buffer[length];
  buffer[0] = 0xFF;        // Frame 1
  buffer[1] = 0xFE;        // Frame 2
  buffer[2] = 0xE6;        // Header byte - Programming on Main - Byte schreiben
  buffer[3] = 0x30;        // Kennung
  buffer[4] = aH;          // Daten 1 - Address High
  buffer[5] = aL;          // Daten 2 - Address Low
  buffer[6] = cvH + 0xE4;  // Daten 3 - CV Address - 2 most significant bits
  buffer[7] = cvL;         // Daten 4 - CV Address - 8 remaining bits
  buffer[8] = cvIntValue;  // Daten 5 - CV value
  buffer[9] = 0x00;        // XOR value
  length = 10;
  // create the checksum (XOR) byte)
  unsigned char myxor = 0;
  for (i=2; i < (length-1) ; i++) { myxor = myxor ^ buffer[i];}
  buffer[length -1] = myxor;      // XOR value
  // send the actual data over the TCP connection for outgoing PoM messages
  [_oStreamPoM write:(const uint8_t *)buffer maxLength:length];
  // to determine if we receive a reply, start the time out timer
  [self startRsTimeOut];
  // Write the value of the POM packet to the send status line
  NSString *message = @"POM verify message send: CV=";
  message = [message stringByAppendingString:[NSString stringWithFormat:@"%d",cvNumber + 1]];
  // message = [self packetToString:buffer withLength:length];
  [_topObject showSendStatus: message];
}


// ****************************************************************************************************************************************************
// ********************************************************* RECEIVE METHODS FOR THE PoM STREAM *******************************************************
// ****************************************************************************************************************************************************
// In the next procedures we first receive a byte from the interface for sending PoM messages. If all bytes for one
// frame are received, we determine what kind of frame it is

- (void)readByteFromPomStream:(NSStream *)stream {
  // this routine receives individual bytes from the input stream, and creates a frame
  NSInteger len = 0;  // to check if we have read exactly 1 byte from the TCP stream
  len = [(NSInputStream *)stream read:pomInStream maxLength:1];
  if (len == 1) {
    // We have an input byte (no error reading from stream)
    // NSLog(@"char = %0x", pomInStream[0]);  // for testing
    totalPomBytes++;
    if (pomSynchronized) {
      pomInBuffer[pomInputBufferSize] = pomInStream[0];
      pomInputBufferSize ++;
      if ([self completePomFrameReceived]) {
        [self handlePomFrame];
        [self expectNextPomFrame];}
    }
    else // we are not sychronized yet. Ignore stream input, unless it has value 0xFF
      if (pomInStream[0] == 0xFF) {
        // this is likely the start of a new frame, although it may be a valid data value as well. Lets try ...
        pomSynchronized = 1;
        pomInBuffer[0] = 0xFF;
        pomInputBufferSize = 1;}
  }
}


- (uint8_t)completePomFrameReceived {
  // this function determines the received frame is complete
  if (pomInputBufferSize < 3) return 0;  // can happen during initialisation
  // calculate what the expected frame size should be
  int command_length = (pomInBuffer[2] & 0b00001111) + 4;
  // check if frame start is what we expect
  if ((pomInBuffer[0] == 0xFF) && ((pomInBuffer[1] == 0xFD) || (pomInBuffer[1] == 0xFE))) {
    if (command_length == pomInputBufferSize) return 1; // Yes, current frame complete
    if (command_length >  pomInputBufferSize) return 0; // Command not yet complete, continue to receive more bytes
  }
  // frame start is not what we expect, so an error. Stop analysing this frame and start from scratch
  else pomSynchronized = 0;
  [self expectNextPomFrame];
  return 0;
}


- (void)handlePomFrame {
  // This procedure analyzes the received frame. In case we use the same Lenz interface for sending PoM messages and
  // receiving RS-bus feedbacks, this method will also be called for every feedback message received. Therefore we have 
  // to select only those messages we expect as feedback from the PoM interface
  switch (_operationalState) {
    case INACTIVE:      {break;}
    case POM_WRITE:     {
      // Check if it is the Lenz Interface handshake, saying command is forwarded to Central Station
      // In that case we can proceed with the next PoM write message. For safety, let's shortly wait before sending
      if ((pomInBuffer[1] == 0xFE) && (pomInBuffer[2] == 0x01) && (pomInBuffer[3] == 0x04)  && (pomInBuffer[4] == 0x05))
        [self stopPomTimeOut];
        [self performSelector:@selector(sendNextPomWritePacketFromQueue) withObject:nil afterDelay:POM_WRITE_DELAY];
      break;}
    case POM_VERIFY:    {
      // Check if it is the Lenz TCP Status Response, saying no error.
      // After a PoM verify timeout, we did send an TCP Status Request message over the first TCP connection,
      // which is used for sending PoM messages. If a response is received (thus we are here) we'll have to send a second
      // Interface Status Request message over the second TCP connection, which is used for RS-Bus feedback messages
      // Note that we may use a single physical Lenz Interface box for both connections. The idea, however, is to test
      // the TCP connections, and not the specific device.
      if (ErrorStatePomVerify == CheckingSendingLenz) { 
        if ((pomInBuffer[1] == 0xFE) && (pomInBuffer[2] == 0xF2) && (pomInBuffer[3] == 0x01) && (pomInBuffer[4] == 0x01)) {
          ErrorStatePomVerify = CheckingReceivingLenz;
          [self sendTcpStatusRequest:_oStreamRS];}
      }
      break;}
    case STATUS_LENZ_1: {if (pomInBuffer[1] == 0xFE) [self queryStatusLenz1]; break;}
    case STATUS_LENZ_2: {break;}
  }
}


- (void)expectNextPomFrame {
  // is called to prepare reception of teh next frame
  pomInputBufferSize = 0;
}


// ****************************************************************************************************************************************************
// ******************************************************* RECEIVE METHODS FOR THE FEEDBACK STREAM ****************************************************
// ****************************************************************************************************************************************************
// In the next procedures we first receive a byte from the Feedback (RS-Bus) interface. If all bytes for one
// frame are received, we determine what kind of frame it is.


- (void)readByteFromRsBus:(NSStream *)stream {
  // this routine receives individual bytes from the input stream, and creates a frame
  NSInteger len = 0;  // to check if we have read exactly 1 byte from the TCP stream
  len = [(NSInputStream *)stream read:rsBusInStream maxLength:1];
  if (len == 1) {
    // We have an input byte (no error reading from stream)
    // NSLog(@"char = %0x", rsBusInStream[0]);  // for testing
    totalRsBusBytes++;
    if (rsBusSynchronized) {
      rsBusInBuffer[rsBusInputBufferSize] = rsBusInStream[0];
      rsBusInputBufferSize ++;
      if ([self completeRsBusFrameReceived]) {
        // [self showRSbuffer];
        [self handleRsBusFrame];
        [self expectNextRsBusFrame];
      }
    }
    else // we are not sychronized yet. Ignore stream input, unless it has value 0xFF
      if (rsBusInStream[0] == 0xFF) {
        // this is likely the start of a new frame, although it may be a valid data value as well. Lets try ...
        rsBusSynchronized = 1;
        rsBusInBuffer[0] = 0xFF;
        rsBusInputBufferSize = 1;}
  }
}


- (uint8_t)completeRsBusFrameReceived {
  // this function determines the received frame is complete
  if (rsBusInputBufferSize < 3) return 0;  // can happen during initialisation
  // calculate what the expected frame size should be
  int command_length = (rsBusInBuffer[2] & 0b00001111) + 4;
  // check if frame start is what we expect
  if ((rsBusInBuffer[0] == 0xFF) && ((rsBusInBuffer[1] == 0xFD) || (rsBusInBuffer[1] == 0xFE))) {
    if (command_length == rsBusInputBufferSize) return 1; // Yes, current frame complete
    if (command_length >  rsBusInputBufferSize) return 0; // Command not yet complete, continue to receive more bytes
  }
  // frame start is not what we expect, so an error. Stop analysing this frame and start from scratch
  rsBusSynchronized = 0;
  [self expectNextRsBusFrame];
  return 0;
}


- (void)handleRsBusFrame {
  // This procedure analyzes the received frame, and determines the kind of frame.
  // It subsequently calls other routines to handle that specific frame:
  // 1: Broadcast message from the Lenz interface containing a RS-Bus Feedback frame => Call handleFeedbackFrame
  // 2: Broadcast message from the Lenz interface containing another frame => Call handleGenericBroadcastFrame
  // 3: Message providing status information regarding Lenz interface => Call handleInterfaceFrame
  // 4: Message containing the specific Interface status Response Frame => Call handleInterfaceStatusResponseFrame
  // 5: Other message: ignore
  switch (_operationalState) {
    case INACTIVE:      {[self expectNextRsBusFrame]; break;}
    case POM_WRITE:     {[self expectNextRsBusFrame]; break;}
    case POM_VERIFY:    {
      // Check if this is a broadcast packet.
      if (rsBusInBuffer[1] == 0xFD) {
        // Check if this is a Feedback message
        if ((rsBusInBuffer[2] & 0b11110000) == 0x40) {
          // In most cases we may expect feedback messages as reaction to PoM verify messages
          // Two cases are possible, however:
          // 1) After sending the PoM verify request no errors occured. In that case we should handle the frame
          // 2) After sending the PoM verify request one or more timeout errors occured. In response we've queried
          // CV1 and CV8, to determine if the decoder listens to this address. At this stage of error processing
          // we may have received the responses for CV1 or CV8. In such cases we may ignore the incomming
          // Feedback message (no need to check its contents), but we should resend the original CV PoM Verify,
          // just to be sure we previously did not have a temporary error.
          if (ErrorStatePomVerify == NoError) [self handleFeedbackFrame];
          else
            if ((ErrorStatePomVerify == CheckingCv1) || (ErrorStatePomVerify == CheckingCv8)) {
              [self stopRsTimeOut];
              ErrorStatePomVerify = RetryCv;
              [self expectNextRsBusFrame];
              [self retryPreviousdPomVerifyPacket];}
        }
        else if (rsBusInBuffer[2] == 0x61) {
          // These are warnings or error messages, which should not have happened
          // Stop further querying CVs
          [self handleGenericBroadcastFrame];
          [self clear_cvs_verify];
          _operationalState = INACTIVE;}
      }
      // check for non-broadcast messages
      if (rsBusInBuffer[1] == 0xFE) {
        // Check for general interface status messages.
        // We may ignore the common "request is send to command station", since we do not this for flow control purposes
        if ((rsBusInBuffer[2] == 0x01) && ((rsBusInBuffer[2] != 0x04))) {[self handleInterfaceFrame];} 
        // Check if it is the Lenz Interface Status Response, saying no error.
        // After a PoM verify timeout, we did send an Interface Status Request message over the first TCP connection,
        // followed by a second Interface Status Request message over the TCP connection for RS-Bus feedback messages
        // If we receive such response, we should continue the error detection procedure by checking whether CV1 exists
        if ((rsBusInBuffer[2] == 0xF2) && (rsBusInBuffer[3] == 0x01) && (rsBusInBuffer[4] == 0x01)) {
          if (ErrorStatePomVerify == CheckingReceivingLenz) {ErrorStatePomVerify = CheckingCv1; [self checkCv1];}
        }
        // Reset input buffer, to receive next packet
        [self expectNextRsBusFrame];
      }
      break;}
    case STATUS_LENZ_1: {[self expectNextRsBusFrame]; break;}
    case STATUS_LENZ_2: {if (rsBusInBuffer[1] == 0xFE) [self queryStatusLenz2]; break;}
  }
}


- (void)expectNextRsBusFrame {
  // is called to prepare reception of teh next frame
  rsBusInputBufferSize = 0;
}


// ****************************************************************************************************************************************************
// ********************************************************* ANALYZE RS-BUS FEEDBACK MESSAGE **********************************************************
// ****************************************************************************************************************************************************
- (void)handleFeedbackFrame {
  uint8_t parity = 0;         // to calculate the parity
  int xor_length;             // determines the number of bytes included in the parity (X-OR) check
  // Check parity
  parity = (uint8_t) rsBusInBuffer[2];
  xor_length = (rsBusInBuffer[2] & 0b00001111) + 4;
  for (int i = 3; i < xor_length; i++) {parity ^= (uint8_t) rsBusInBuffer[i];}
  if (parity) {[self expectNextRsBusFrame];; return;} // Parity error
  // Fill in the RS-bus fields. Note that a single xpressbus feedback packet may include upto 7 feedback messages
  // We'll handle them one by one, in a relative simple way
  // Message 1
  [self DetermineFeedbackValueForAddressByte:rsBusInBuffer[3] andNibbleByte:rsBusInBuffer[4]];
  // Message 2
  if (rsBusInputBufferSize < 7) {[self expectNextRsBusFrame]; return;};
  [self DetermineFeedbackValueForAddressByte:rsBusInBuffer[5] andNibbleByte:rsBusInBuffer[6]];
  // Message 3
  if (rsBusInputBufferSize < 9) {[self expectNextRsBusFrame]; return;};
  [self DetermineFeedbackValueForAddressByte:rsBusInBuffer[7] andNibbleByte:rsBusInBuffer[8]];
  // Message 4
  if (rsBusInputBufferSize < 11) {[self expectNextRsBusFrame]; return;};
  [self DetermineFeedbackValueForAddressByte:rsBusInBuffer[9] andNibbleByte:rsBusInBuffer[10]];
  // Message 5
  if (rsBusInputBufferSize < 13) {[self expectNextRsBusFrame]; return;};
  [self DetermineFeedbackValueForAddressByte:rsBusInBuffer[11] andNibbleByte:rsBusInBuffer[12]];
  // Message 6
  if (rsBusInputBufferSize < 15) {[self expectNextRsBusFrame]; return;};
  [self DetermineFeedbackValueForAddressByte:rsBusInBuffer[13] andNibbleByte:rsBusInBuffer[13]];
  // Message 7
  if (rsBusInputBufferSize < 17) {[self expectNextRsBusFrame]; return;};
  [self DetermineFeedbackValueForAddressByte:rsBusInBuffer[15] andNibbleByte:rsBusInBuffer[16]];
  [self expectNextRsBusFrame];
}


- (void)DetermineFeedbackValueForAddressByte:(int)byte1 andNibbleByte:(int)byte2 {
  // Switch and similar decoders send their feedback messages ALWAYS via RS-Address 128
  // Should stop the RsTimeOut if 1) address is OK, nibble1 received as well as nibble2
  uint8_t ReceivedAddress = byte1 + 1;
  // we listen to RS address 128
  if (ReceivedAddress == 128) {
    uint8_t value   = (byte2 & 0b00001111);
    uint8_t nibble  = (byte2 & 0b00010000) >> 4;
    if (nibble == 0){
      if (waiting_for_second_nibble == 0) {
        // this is the normal case
        nibble1Value = value;
        waiting_for_second_nibble = 1;}
      else {
        // This is an error. Seems we lost the second nibble. Do the same as above
        nibble1Value = value;
        waiting_for_second_nibble = 1;}
      [self expectNextRsBusFrame];
    }
    else {
      if (waiting_for_second_nibble == 1) {
        // this is the normal case. We have now received the complete data
        [self stopRsTimeOut];
        numberOfVerifyReceived ++;
        uint8_t totalnibble = (value << 4) + nibble1Value;
        // Inform the AppDelegate we've received a feedback message
        [_topObject feedbackPacketReceivedForAddress:ReceivedAddress withCV:previousVerifyCv withValue:totalnibble];
        // NSLog(@"Address:%d - CV:%d - Value:%d, ", ReceivedAddress, previousVerifyCv, totalnibble);
        // Update the receive status line
        NSString *message = @"Feedback received. Address=";
        message = [message stringByAppendingString:[NSString stringWithFormat:@"%d",ReceivedAddress]];
        message = [message stringByAppendingString:[NSString stringWithFormat:@" CV="]];
        message = [message stringByAppendingString:[NSString stringWithFormat:@"%d",previousVerifyCv]];
        message = [message stringByAppendingString:[NSString stringWithFormat:@" Value="]];
        message = [message stringByAppendingString:[NSString stringWithFormat:@"%d",totalnibble]];
        [_topObject showReceiveStatus: message];
        // Continue with next request 
        waiting_for_second_nibble = 0;
        [self expectNextRsBusFrame];
        [self sendNextPomVerifyPacketFromQueue];
      }
      else {
        // This is an error. Seems we lost the first nibble. Skip data and time out
        waiting_for_second_nibble = 0;}
      [self expectNextRsBusFrame];
    }
  }
}


// ****************************************************************************************************************************************************
// ************************************ ANALYZE NON-BROADCAST AND INTERFACE MESSAGES FOR THE FEEDBACK CONNECTION **************************************
// ****************************************************************************************************************************************************
- (void)handleGenericBroadcastFrame {
  // Create a status message
  NSString *message = @"Message received from command station for feedbacks: ";
  if ((rsBusInBuffer[3] == 0x00) & (rsBusInBuffer[4] == 0x61)) message = [message stringByAppendingString:@" All off"];
  else if ((rsBusInBuffer[3] == 0x01) & (rsBusInBuffer[4] == 0x60)) message = [message stringByAppendingString:@" All on"];
  else if ((rsBusInBuffer[3] == 0x02) & (rsBusInBuffer[4] == 0x63)) message = [message stringByAppendingString:@" Programming (service) mode"];
  else message = [message stringByAppendingString:[self packetToString:rsBusInBuffer withLength:rsBusInputBufferSize]];
  // write the status message
  [_topObject showReceiveStatus: message];
  [self expectNextRsBusFrame];
}

- (void)handleInterfaceFrame {
  // Create a status message
  NSString *message = @"Message received from Lenz interface for feedbacks: ";
  if ((rsBusInBuffer[3] == 0x01) & (rsBusInBuffer[4] == 0x00)) message = [message stringByAppendingString:@" The number of bytes indicated in the header does not match the number of received bytes"];
  if ((rsBusInBuffer[3] == 0x02) & (rsBusInBuffer[4] == 0x03)) message = [message stringByAppendingString:@" Error between interface and command station (Timeout of transmission from interface to command station"];
  if ((rsBusInBuffer[3] == 0x03) & (rsBusInBuffer[4] == 0x02)) message = [message stringByAppendingString:@" Unknown error (command station sends the interface unexpected requests"];
  if ((rsBusInBuffer[3] == 0x04) & (rsBusInBuffer[4] == 0x05)) message = [message stringByAppendingString:@" Request is send from interface to command station"];
  if ((rsBusInBuffer[3] == 0x05) & (rsBusInBuffer[4] == 0x04)) message = [message stringByAppendingString:@" Command station no longer connects to interface"];
  if ((rsBusInBuffer[3] == 0x06) & (rsBusInBuffer[4] == 0x07)) message = [message stringByAppendingString:@" Buffer overflow in interface"];
  if ((rsBusInBuffer[3] == 0x07) & (rsBusInBuffer[4] == 0x06)) message = [message stringByAppendingString:@" Command station is reconnected to interface"];
  if ((rsBusInBuffer[3] == 0x08) & (rsBusInBuffer[4] == 0x09)) message = [message stringByAppendingString:@" Currently not possible to send requests from interface to command station"];
  if ((rsBusInBuffer[3] == 0x09) & (rsBusInBuffer[4] == 0x08)) message = [message stringByAppendingString:@" Error in the request packet (for example wrong decoder address"];
  if ((rsBusInBuffer[3] == 0x0A) & (rsBusInBuffer[4] == 0x0B)) message = [message stringByAppendingString:@" Unknown eroror (command station does not send the expected reply"];
  // write the status message
  [_topObject showReceiveStatus: message];
  [self expectNextRsBusFrame];
}


// ****************************************************************************************************************************************************
// ************************************************************** TIMEOUT RELATED ROUTINES ************************************************************
// ****************************************************************************************************************************************************
- (void)startPomTimeOut {[self performSelector:@selector(PomTimeOut) withObject:nil afterDelay:POM_TIMEOUT];}
- (void)startRsTimeOut {[self performSelector:@selector(RsTimeOut) withObject:nil afterDelay:RS_BUS_TIMEOUT];}
- (void)stopPomTimeOut {[NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(PomTimeOut) object:nil];}
- (void)stopRsTimeOut {[NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(RsTimeOut) object:nil];}


- (void)PomTimeOut {
  [_topObject showGeneralStatus: @"Fatal error: Lenz Interface for Sending PoM messages no longer responding"];
  [self closeTcp];
}


- (void)RsTimeOut {
  // The timer is set by sendPomVerifyPacketForCV
  // NSLog(@"RsTimeOut");
  waiting_for_second_nibble = 0;
  //  [self sendNextPomVerifyPacketFromQueue];
  switch (ErrorStatePomVerify) {
    case NoError:                 { // Check TCP connection to sending Lenz interface
      ErrorStatePomVerify = CheckingSendingLenz;
      [self sendTcpStatusRequest:_oStreamPoM];
      break;}
    case CheckingSendingLenz:     { break;} // We test the PoM connection, so we should not get a time_out on the feedback connection
    case CheckingReceivingLenz:   { // No RS-Bus feedbacks can be received.
      ErrorStatePomVerify = NoFeedbacks;
      [_topObject showGeneralStatus: @"TCP connection to Lenz interface for (RS-Bus) feedback messages lost"];
      [self closeTcpRs];
      [self clear_cvs_verify];
      break;}
    case CheckingCv1:             { // If we test CV1 and get a time-out, we'll try again but now with CV8
      ErrorStatePomVerify = CheckingCv8;
      [self checkCv8];
      break;}
    case CheckingCv8:             { // If we also have a timeout on CV8, we must conclude no decoder is listening to this address
      ErrorStatePomVerify = NoError;
      // Before september 2020 we had the next line:
      // [_topObject showGeneralStatus: @"No decoder responds to this address"];
      // September 2020 we changed this into the following 4 lines:
      int dec_adr = [_topObject.dccDecoderObject decoderAddress];
      if (dec_adr < 0) {
        [_topObject showGeneralStatus: @"No non-initialised decoders found"];
      }
      else {
        NSString * message = @"No decoder responds to address: ";
        message = [message stringByAppendingString:[NSString stringWithFormat:@"%d", dec_adr]];
        [_topObject showGeneralStatus:message];
      }
      // Stop the progress indicator
      [_topObject progressIndicator: 0];
      [self clear_cvs_verify];
      break;}
    case RetryCv:                 { // Seems the CV is not implemented. Forget about this CV and continue with next CV
      ErrorStatePomVerify = NoError;
      [_topObject showGeneralStatus: @"CV not implemented"];
      [self sendNextPomVerifyPacketFromQueue];
      break;}
    case NoFeedbacks:             { break;}
    case NoCommunicationPossible: { break;}
  }
}


- (void)checkCv1 {
  // NSLog(@"checkCv1. ErrorStatePomVerify=%d", ErrorStatePomVerify);
  [self sendPomVerifyPacketForCV:1];
}

- (void)checkCv8 {
  // NSLog(@"checkCv8. ErrorStatePomVerify=%d", ErrorStatePomVerify);
  [self sendPomVerifyPacketForCV:8];
}


// ****************************************************************************************************************************************************
// *********************************************************** SEND METHODS FOR TESTING STATUS ********************************************************
// ****************************************************************************************************************************************************
// The next routine is used to react upon RS_Bus feedback time outs, to see if the Lenz interfaces are still reachable
- (void)sendTcpStatusRequest:(NSOutputStream *)stream {
  uint8_t buffer[5];
  buffer[0]=0xFF;  buffer[1]=0xFE; buffer[2]=0xF1; buffer[3]=0x01;  buffer[4]=0xF0;
  [stream write:(const uint8_t *)buffer maxLength:5];
}

// All subsequent routines are used to determine general status info of the Lenz interfaces
- (void)sendInterfaceVersionAndCodeRequest:(NSOutputStream *)stream {
  uint8_t buffer[4];
  buffer[0]=0xFF;  buffer[1]=0xFE; buffer[2]=0xF0; buffer[3]=0xF0;
  [stream write:(const uint8_t *)buffer maxLength:4];
}

- (void)sendInterfaceXpressnetAddressRequest:(NSOutputStream *)stream {
  uint8_t buffer[6];
  buffer[0]=0xFF;  buffer[1]=0xFE; buffer[2]=0xF2; buffer[3]=0x01;  buffer[4]=0x00; buffer[5]=0x02;
  [stream write:(const uint8_t *)buffer maxLength:6];
}

- (void)sendInterfaceXpressnetVersionRequest:(NSOutputStream *)stream {
  uint8_t buffer[5];
  buffer[0]=0xFF;  buffer[1]=0xFE; buffer[2]=0xF1; buffer[3]=0x02;  buffer[4]=0xF3;
  [stream write:(const uint8_t *)buffer maxLength:5];
}

- (void)sendInterfaceFreeTcpRequest:(NSOutputStream *)stream {
  uint8_t buffer[5];
  buffer[0]=0xFF;  buffer[1]=0xFE; buffer[2]=0xF1; buffer[3]=0x03;  buffer[4]=0xF2;
  [stream write:(const uint8_t *)buffer maxLength:5];
}


- (void)queryStatusLenz1{
  // We will only query if nothing else is happening. Multiple parameters will be queried
  // The operational status will change until all parameters have been retrieved
  // We will subsequently query:
  // 1) Software version of the Lenz interface
  // 2) Xpressnet address
  // 3) Xpressnet version
  // 4) Number of TCP connections the Lenz interface is still willing to accept
  if (_operationalState == INACTIVE) {
    _operationalState = STATUS_LENZ_1;
    [self sendInterfaceVersionAndCodeRequest:_oStreamPoM];
  }
  else if (_operationalState == STATUS_LENZ_1) {
    // 1: sendInterfaceVersionAndCodeRequest => sendInterfaceXpressnetAddressRequest
    if (pomInBuffer[2] == 0x02) {
      lenzStatusPom.softwareVersionMain  = (pomInBuffer[3] & 0b11110000) >> 4;
      lenzStatusPom.softwareVersionMinor = pomInBuffer[3] & 0b00001111;
      lenzStatusPom.softwareCode = pomInBuffer[4];
      [self sendInterfaceXpressnetAddressRequest:_oStreamPoM];
    }
    // 2: sendInterfaceXpressnetAddressRequest => sendInterfaceXpressnetVersionRequest
    else if ((pomInBuffer[2] == 0xF2) && (pomInBuffer[3] == 0x01) && (pomInBuffer[4] > 0x01)) {
      lenzStatusPom.xpressnetAddress = pomInBuffer[4];
      [self sendInterfaceXpressnetVersionRequest:_oStreamPoM];
    }
    // 3: sendInterfaceXpressnetVersionRequest => sendInterfaceFreeTcpRequest
    else if ((pomInBuffer[2] == 0xF2) && (pomInBuffer[3] == 0x02)) {
      lenzStatusPom.xpressnetVersionMain  = (pomInBuffer[4] & 0b11110000) >> 4;
      lenzStatusPom.xpressnetVersionMinor = pomInBuffer[4] & 0b00001111;
      [self sendInterfaceFreeTcpRequest:_oStreamPoM];
    }
    // 4: sendInterfaceFreeTcpRequest => READY
    else if ((pomInBuffer[2] == 0xF2) && (pomInBuffer[3] == 0x03)) {
      lenzStatusPom.tcpConnectionsFree = pomInBuffer[4];
      _operationalState = INACTIVE;
      // NSLog(@"Lenz PoM Interface:");
      // NSLog(@"Software Version=%d.%d; Code=%d", lenzStatusPom.softwareVersionMain, lenzStatusPom.softwareVersionMinor, lenzStatusPom.softwareCode);
      // NSLog(@"Xpressnet address: %d", lenzStatusPom.xpressnetAddress);
      // NSLog(@"Xpressnet version=%d.%d",lenzStatusPom.xpressnetVersionMain, lenzStatusPom.xpressnetVersionMinor);
      // NSLog(@"Number of available TCP connections=%d",lenzStatusPom.tcpConnectionsFree);
    }
    // Something went wrong. Stop
    else {_operationalState = INACTIVE;}
  }
}


- (void)queryStatusLenz2{
  // Same logic as above
  if (_operationalState == INACTIVE) {
    _operationalState = STATUS_LENZ_2;
    [self expectNextRsBusFrame]; 
    [self sendInterfaceVersionAndCodeRequest:_oStreamRS];
  }
  else if (_operationalState == STATUS_LENZ_2) {
    // 1: sendInterfaceVersionAndCodeRequest => sendInterfaceXpressnetAddressRequest
    if (rsBusInBuffer[2] == 0x02) {
      lenzStatusRS.softwareVersionMain  = (rsBusInBuffer[3] & 0b11110000) >> 4;
      lenzStatusRS.softwareVersionMinor = rsBusInBuffer[3] & 0b00001111;
      lenzStatusRS.softwareCode = rsBusInBuffer[4];
      [self expectNextRsBusFrame]; 
      [self sendInterfaceXpressnetAddressRequest:_oStreamRS];
    }
    // 2: sendInterfaceXpressnetAddressRequest => sendInterfaceXpressnetVersionRequest
    else if ((rsBusInBuffer[2] == 0xF2) && (rsBusInBuffer[3] == 0x01) && (rsBusInBuffer[4] > 0x01)) {
      lenzStatusRS.xpressnetAddress = rsBusInBuffer[4];
      [self expectNextRsBusFrame]; 
      [self sendInterfaceXpressnetVersionRequest:_oStreamRS];
    }
    // 3: sendInterfaceXpressnetVersionRequest => sendInterfaceFreeTcpRequest
    else if ((rsBusInBuffer[2] == 0xF2) && (rsBusInBuffer[3] == 0x02)) {
      lenzStatusRS.xpressnetVersionMain  = (rsBusInBuffer[4] & 0b11110000) >> 4;
      lenzStatusRS.xpressnetVersionMinor = rsBusInBuffer[4] & 0b00001111;
      [self expectNextRsBusFrame]; 
      [self sendInterfaceFreeTcpRequest:_oStreamRS];
    }
    // 4: sendInterfaceFreeTcpRequest => READY
    else if ((rsBusInBuffer[2] == 0xF2) && (rsBusInBuffer[3] == 0x03)) {
      lenzStatusRS.tcpConnectionsFree = rsBusInBuffer[4];
      [self expectNextRsBusFrame]; 
      _operationalState = INACTIVE;
      // NSLog(@"Lenz RS-bus Interface:");
      // NSLog(@"Software Version=%d.%d; Code=%d", lenzStatusRS.softwareVersionMain, lenzStatusRS.softwareVersionMinor, lenzStatusRS.softwareCode);
      // NSLog(@"Xpressnet address: %d", lenzStatusRS.xpressnetAddress);
      // NSLog(@"Xpressnet version=%d.%d",lenzStatusRS.xpressnetVersionMain, lenzStatusRS.xpressnetVersionMinor);
      // NSLog(@"Number of available TCP connections=%d",lenzStatusRS.tcpConnectionsFree);
    }
    // Something went wrong. Stop
    else {[self expectNextRsBusFrame]; _operationalState = INACTIVE;}
  }
}


// ****************************************************************************************************************************************************
// ************************************************************ (TEMPORARY) TESTING PROCEDURE *********************************************************
// ****************************************************************************************************************************************************
- (NSString *) packetToString:(uint8_t *)buffer withLength:(NSUInteger)length {
  // Convert the hex value of a DCC packet into a string.
  unsigned int i;
  NSString *message = @"";
  for (i=0; i < (length) ; i++) {message = [message stringByAppendingString:[NSString stringWithFormat:@" %X", buffer[i]]];}
  return message;
}


- (void)showRSbuffer {
  NSString *message = @"RSBuffer:  ";
  message = [message stringByAppendingString:[self packetToString:rsBusInBuffer withLength:rsBusInputBufferSize]];
  [_topObject showSendStatus: message];
}


@end
