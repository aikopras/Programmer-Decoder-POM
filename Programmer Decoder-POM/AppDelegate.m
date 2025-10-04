//
//  AppDelegate.m
//  CV Editor for switch and relays decoders based on OPENDECODER22
//
//  Created by Aiko Pras on 26-02-12 / 11-03-2013 / 06-04-2014
//  Copyright (c) 2013,2014 by Aiko Pras. All rights reserved.
//

#import "AppDelegate.h"
#import "DCCDecoder.h"
#import "TCPConnection.h"
#import "PreferencesController.h"

@implementation AppDelegate

// Main objects
@synthesize dccDecoderObject             = _dccDecoderObject;
@synthesize tcpConnectionsObject         = _tcpConnectionsObject;
@synthesize preferencesController        = _preferencesController;
// Main user interface window plus tab part
@synthesize window                       = _window;
@synthesize windowTabs                   = _windowTabs;
// General properties common to all decoders
@synthesize address                      = _address;
@synthesize DecoderVendor                = _DecoderVendor;
@synthesize DecoderVersion               = _DecoderVersion;
@synthesize DecoderType                  = _DecoderType;
@synthesize DecoderSubType               = _DecoderSubType;
@synthesize DecoderErrors                = _DecoderErrors;
@synthesize ledOn                        = _ledOn;
//
// Control tab properties
@synthesize comboRsRetry                 = _comboRsRetry;
@synthesize comboCmdSystem               = _comboCmdSystem;
@synthesize comboDecType                 = _comboDecType;
@synthesize buttonControlTabDefaults     = _buttonControlTabDefaults;
@synthesize buttonControlTabGet          = _buttonControlTabGet;
@synthesize buttonControlTabSet          = _buttonControlTabSet;
//
// Switch tab properties
@synthesize cv3Text                      = _cv3Text;
@synthesize cv4Text                      = _cv4Text;
@synthesize cv5Text                      = _cv5Text;
@synthesize cv6Text                      = _cv6Text;
@synthesize cv3Slider                    = _cv3Slider;
@synthesize cv4Slider                    = _cv4Slider;
@synthesize cv5Slider                    = _cv5Slider;
@synthesize cv6Slider                    = _cv6Slider;
@synthesize comboSkipEven                = _comboSkipEven;
@synthesize comboSendFB                  = _comboSendFB;
@synthesize comboAlwaysAct               = _comboAlwaysAct;
@synthesize buttonSwitchTabDefaults    = _buttonSwitchTabDefaults;
@synthesize buttonSwitchTabGet         = _buttonSwitchTabGet;
@synthesize buttonSwitchTabSet         = _buttonSwitchTabSet;
//
// RS-Bus tab properties
@synthesize comboRsFec                 = _comboRsFec;
@synthesize buttonRsBusTabSet          = _buttonRsBusTabSet;
//
// Initialise tab properties
@synthesize addressNew                   = _addressNew;
@synthesize iniTabDecoderAddress         = _iniTabDecoderAddress;
@synthesize iniTabRsAddress              = _iniTabRsAddress;
@synthesize buttonIniTabSet              = _buttonIniTabSet;
//
// Status bar properties
@synthesize connectionStatus             = _connectionStatus;
@synthesize sendStatus                   = _sendStatus;
@synthesize receiveStatus                = _receiveStatus;
@synthesize progressIndicator            = _progressIndicator;

// ************************************************************************************************************
// ******************************************** DEFINE CV MAPPINGS ********************************************
// ************************************************************************************************************
#define myAddrL        1
#define T_on_F1        3
#define T_on_F2        4
#define T_on_F3        5
#define T_on_F4        6
#define version        7
#define VID            8
#define myAddrH        9
#define myRSAddr      10
#define CmdStation    19
#define RSRetry       20
#define SkipEven      21
#define Search        23
#define PoMStart      24
#define Restart       25
#define DccQuality    26
#define DecType       27
#define BiDi          28
#define Config        29
#define VID_2         30
#define SendFB        33
#define AlwaysAct     34


// ************************************************************************************************************
// ********************************************** INITIALIZATION **********************************************
// ************************************************************************************************************
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
  // Step 1: dccDecoderObject
  // Allocate memory and initialise a new instance of the DCCDecoder class
  _dccDecoderObject = [[DCCDecoderClass alloc] init];
  [_dccDecoderObject initialise];
  // Step 2: preferences
  // Check if the preferences file exists. If not, create it
  [self checkPreferences];
  // Step 3: tcpConnectionsObject
  // Allocate memory and initialise a new instance of the TCPConnection class
  _tcpConnectionsObject = [[TCPConnectionsClass alloc] init];
  // Now open the TCP connection
  [_tcpConnectionsObject openTcp];
  // Step 4: UI tabs
  [self updateTabs];
  // Step 5: initialise progress indicator
  [_progressIndicator setDisplayedWhenStopped: NO];
  // Step 6: Added in September 2020
  // Try if there are non-initilised decoders. If there are, we use the "version" CV to select
  // the appropriate algorithm to determine the value of CV1 during initialisation
  // Set the address of the uninitilised decoder to -1, which results in a PoM message with address 6999
  // Since reading CV values take time, we first read the "version" CV.
  [_dccDecoderObject setDecoderAddress:-1];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:version];
  [_tcpConnectionsObject sendNextPomVerifyPacketFromQueue];
  // But in addition we read all other CVs to allow the appropriate TABs (switch, servo, ...) to be displayed.
  [self readAllCvs];
  [self updateTabs];
}

- (void) updateTabs {
  [self updateMainWindow];
  [self updateControlTab];
  [self updateRsBusTab];
  [self updateSwitchTab];
  [self updateIniTab];
  [self updateCvTab];
  // Depending on decoder type, show SWITCH tab
  if ([_dccDecoderObject getCv:DecType] != 16) [self hideSwitchTab];
  if ([_dccDecoderObject getCv:DecType] == 16) {[self showSwitchTab]; [self updateSwitchTab];}
}

// ************************************************************************************************************
// ********************************** FEEDBACK MESSAGE RECEIVED BY THE TCP OBJECT *****************************
// ************************************************************************************************************
- (void)feedbackPacketReceivedForAddress:(int)rsBusAddress withCV:(int)cvNumber withValue:(u_int8_t)cvValue {
  // NSLog(@"Feedback received. Address:%d CV:%d Value:%d", rsBusAddress, cvNumber, cvValue);
  [_dccDecoderObject setCv:cvNumber withValue:cvValue];
  [self updateTabs];
}

- (void)sendNextPomVerifyPacketFromQueueCompleted{
  // NSLog(@"All PoM verify packets are send");
}

- (void)sendNextPomWritePacketFromQueueCompleted{
  // NSLog(@"All PoM write packets are send");
}

// ************************************************************************************************************
// ************************************* USER INTERFACE METHODS - MAIN WINDOW *********************************
// ************************************************************************************************************
- (IBAction)takeDecoderAddressFrom:(id)sender {
  // NSLog(@"User input - Decoder address: %i", [sender intValue]);
  int newValue = [sender intValue];
  if (newValue < 1) {newValue = 1;}
  if (newValue > 1024) {newValue = 1024;}
  newValue = newValue - 1;   // entering is 1..1024 <-> storing is 0..1023
  newValue = newValue >> 2;   // decoder address = switch address DIV 4
  [_dccDecoderObject setDecoderAddress:newValue];
}

- (IBAction)selectAddressPushed:(id)sender {
  [self readLastInputFromTextFields];
  [self readAllCvs];
}

- (IBAction)ledOnPushed:(id)sender {
  [self readLastInputFromTextFields];
  if ([_ledOn state]) {
    [_dccDecoderObject setCv:Search withValue:1];
    [_tcpConnectionsObject queuePomWritePacketForCV:Search];
    [_tcpConnectionsObject sendNextPomWritePacketFromQueue];
    [self setButtonTitleFor:_ledOn toString:@"LED ON" withColor:[NSColor redColor]];
  }
  else {
    [_dccDecoderObject setCv:Search withValue:0];
    [_tcpConnectionsObject queuePomWritePacketForCV:Search];
    [_tcpConnectionsObject sendNextPomWritePacketFromQueue];
    [self setButtonTitleFor:_ledOn toString:@"LED ON" withColor:[NSColor blackColor]];
  }
}

- (IBAction)restartPushed:(id)sender {
  [self readLastInputFromTextFields];
  [_dccDecoderObject setCv:Restart withValue:0x0D];
  [_tcpConnectionsObject queuePomWritePacketForCV:Restart];
  [_tcpConnectionsObject sendNextPomWritePacketFromQueue];
  // Read all CVs again, after a delay of 0.3 seconds
  [self performSelector:@selector(readAllCvs) withObject:nil afterDelay:0.3];
}

- (void)readAllCvs{
  uint16_t cvNumber;
  [_tcpConnectionsObject queuePomVerifyPacketForCV:1];
  for (cvNumber= 3; cvNumber <= 10; cvNumber++) [_tcpConnectionsObject queuePomVerifyPacketForCV:cvNumber];
  for (cvNumber=19; cvNumber <= 21; cvNumber++) [_tcpConnectionsObject queuePomVerifyPacketForCV:cvNumber];
  for (cvNumber=23; cvNumber <= 30; cvNumber++) [_tcpConnectionsObject queuePomVerifyPacketForCV:cvNumber];
  for (cvNumber=33; cvNumber <= 34; cvNumber++) [_tcpConnectionsObject queuePomVerifyPacketForCV:cvNumber];
  [_tcpConnectionsObject sendNextPomVerifyPacketFromQueue];
  // Remove possible red color from SET buttons
  [self colorGetButtonControlTab:0];
  [self colorGetButtonSwitchTab:0];
  [self colorGetButtonIniTab:0];
}

- (void) updateMainWindow {
  [_DecoderVendor  setObjectValue:[_dccDecoderObject DecoderVendor]];
  [_DecoderVersion setObjectValue:[_dccDecoderObject DecoderVersion]];
  [_DecoderType    setObjectValue:[_dccDecoderObject DecoderType]];
  [_DecoderSubType setObjectValue:[_dccDecoderObject DecoderSubType]];
  [_DecoderErrors  setObjectValue:[_dccDecoderObject DecoderErrors]];
  if ([_dccDecoderObject getCv:23])   [self setButtonTitleFor:_ledOn toString:@"LED ON" withColor:[NSColor redColor]];
  else                                [self setButtonTitleFor:_ledOn toString:@"LED ON" withColor:[NSColor blackColor]];
}


// ************************************************************************************************************
// ************************** METHODS FOR STATUS LINE AND PROGRESS INDICATOR **********************************
// ************************************************************************************************************
- (void)showGeneralStatus:(NSString *) statustext {[_connectionStatus setObjectValue:statustext];}
- (void)showSendStatus:   (NSString *) statustext {[_sendStatus       setObjectValue:statustext];}
- (void)showReceiveStatus:(NSString *) statustext {[_receiveStatus    setObjectValue:statustext];}


- (void)progressIndicator:(BOOL) activity {
  if (activity == YES) [_progressIndicator startAnimation: self];
  if (activity == NO)  [_progressIndicator stopAnimation: self];
}

- (void)showStatusLineForCV:(int)cvNumber withValue:(float)cvValue {
  // NOTE: FUNCTION IS CURRENTLY NOT BEING USED
  int adrInt = [_dccDecoderObject getSwitchAddress];
  int cvIntValue = cvValue;
  NSString * message = @"Address = ";
  message = [message stringByAppendingString:[NSString stringWithFormat:@"%d", adrInt]];
  message = [message stringByAppendingString:[NSString stringWithFormat:@" / CV "]];
  message = [message stringByAppendingString:[NSString stringWithFormat:@"%d", cvNumber]];
  message = [message stringByAppendingString:[NSString stringWithFormat:@" = "]];
  message = [message stringByAppendingString:[NSString stringWithFormat:@"%d", cvIntValue]];
  [self showGeneralStatus:message];
}


// ************************************************************************************************************
// ************************************* USER INTERFACE METHODS - CONTROL TAB *********************************
// ************************************************************************************************************
- (IBAction)selectedRsRetry:(id)sender {
  uint8_t newValue = [sender intValue];
  if (newValue < 0) {newValue = 0;}
  if (newValue > 2) {newValue = 2;}
  [_dccDecoderObject setCv:RSRetry withValue:newValue];
  [self colorGetButtonControlTab:1];
}

- (IBAction)selectedCmdSystem:(id)sender {
  NSString *newString = [sender stringValue];
  uint8_t newValue = 1; // Select the default value
  if ([newString isEqualToString:@"standard"]) {newValue = 0;} // so not the default
  if ([newString isEqualToString:@"Lenz"])     {newValue = 1;} // Lenz is the default
  [_dccDecoderObject setCv:CmdStation withValue:newValue];
  [self colorGetButtonControlTab:1];
}

- (IBAction)selectedDecType:(id)sender {
  NSString *newString = [sender stringValue];
  uint8_t newValue = 16; // Select the default value
  if ([newString isEqualToString:@"switch"])      {newValue = 16;}
  if ([newString isEqualToString:@"switch plus"]) {newValue = 17;}
  if ([newString isEqualToString:@"servo-2"])     {newValue = 20;}
  if ([newString isEqualToString:@"servo-3"])     {newValue = 21;}
  if ([newString isEqualToString:@"relays-4"])    {newValue = 32;}
  if ([newString isEqualToString:@"relays-16"])   {newValue = 33;}
  if ([newString isEqualToString:@"safety"])      {newValue = 128;}
  [_dccDecoderObject setCv:DecType withValue:newValue];
  [self colorGetButtonControlTab:1];
  [self updateTabs];
}

- (IBAction)pushedControlTabDefaults:(id)sender {
  [self readLastInputFromTextFields];
  [_dccDecoderObject setCv:RSRetry withValue:0];
  [_dccDecoderObject setCv:CmdStation withValue:1];
  // Note: current versions of the SWITCH / RELAYS4 decoders do not support extension boards, and can therefore not be changed
  // [_dccDecoderObject setCv:DecType withValue:16];
  [self colorGetButtonControlTab:1];
  [self updateTabs];
}

- (IBAction)pushedControlTabGet:(id)sender {
  [self readLastInputFromTextFields];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:RSRetry];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:CmdStation];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:DecType];
  [_tcpConnectionsObject sendNextPomVerifyPacketFromQueue];
  [self colorGetButtonControlTab:0];
}

- (IBAction)pushedControlTabSet:(id)sender {
  [self readLastInputFromTextFields];
  [_tcpConnectionsObject queuePomWritePacketForCV:RSRetry];
  [_tcpConnectionsObject queuePomWritePacketForCV:CmdStation];
  // [_tcpConnectionsObject queuePomWritePacketForCV:DecType];
  [_tcpConnectionsObject sendNextPomWritePacketFromQueue];
  [self colorGetButtonControlTab:0];
}

- (void) updateControlTab {
  NSString *newString;
  // RsRetry
  [_comboRsRetry setIntValue:[_dccDecoderObject getCv:RSRetry]];
  // Command System
  newString = @"Lenz";      // default
  if ([_dccDecoderObject getCv:CmdStation] == 0) newString = @"standard";
  if ([_dccDecoderObject getCv:CmdStation] == 1) newString = @"Lenz";
  [_comboCmdSystem selectItemWithObjectValue: newString];
  // DecType
  newString = @"switch";   // default
  if ([_dccDecoderObject getCv:DecType] == 17)  newString = @"switch plus";
  if ([_dccDecoderObject getCv:DecType] == 20)  newString = @"servo-2";
  if ([_dccDecoderObject getCv:DecType] == 21)  newString = @"servo-3";
  if ([_dccDecoderObject getCv:DecType] == 24)  newString = @"lift";
  if ([_dccDecoderObject getCv:DecType] == 32)  newString = @"relays-4";
  if ([_dccDecoderObject getCv:DecType] == 33)  newString = @"relays-16";
  if ([_dccDecoderObject getCv:DecType] == 64)  newString = @"function";
  if ([_dccDecoderObject getCv:DecType] == 128) newString = @"safety";
  [_comboDecType selectItemWithObjectValue: newString];
}

- (void)colorGetButtonControlTab:(int)isRed {
  if (isRed) [self setButtonTitleFor:_buttonControlTabSet toString:@"SET" withColor:[NSColor redColor]];
    else     [self setButtonTitleFor:_buttonControlTabSet toString:@"SET" withColor:[NSColor blackColor]];
}


// ************************************************************************************************************
// ************************************* USER INTERFACE METHODS - RS-BUS TAB **********************************
// ************************************************************************************************************
- (IBAction)selectedRsFec:(id)sender {
  uint8_t newValue = [sender intValue];
  if (newValue < 0) {newValue = 0;}
  if (newValue > 2) {newValue = 2;}
  [_dccDecoderObject setCv:RSRetry withValue:newValue];
  [self colorGetButtonRsBusTab:1];
}

- (void) updateRsBusTab {
  // RSFEC
  [_comboRsFec setIntValue:[_dccDecoderObject getCv:RSRetry]];
}

- (void)colorGetButtonRsBusTab:(int)isRed {
  if (isRed) [self setButtonTitleFor:_buttonRsBusTabSet toString:@"SET" withColor:[NSColor redColor]];
    else     [self setButtonTitleFor:_buttonRsBusTabSet toString:@"SET" withColor:[NSColor blackColor]];
}


// ************************************************************************************************************
// ************************************ USER INTERFACE METHODS - SWITCH TAB ********************************
// ************************************************************************************************************
- (IBAction)selectedCv3:(id)sender {[self storeSwitchDelayCv:T_on_F1 with:[sender floatValue]];}
- (IBAction)selectedCv4:(id)sender {[self storeSwitchDelayCv:T_on_F2 with:[sender floatValue]];}
- (IBAction)selectedCv5:(id)sender {[self storeSwitchDelayCv:T_on_F3 with:[sender floatValue]];}
- (IBAction)selectedCv6:(id)sender {[self storeSwitchDelayCv:T_on_F4 with:[sender floatValue]];}

- (IBAction)selectedSkipEven:(id)sender {
    NSString *newString = [sender stringValue];
    uint8_t newValue = 1; // Select the default value
    if ([newString isEqualToString:@"No"])  {newValue = 0;}
    if ([newString isEqualToString:@"Yes"]) {newValue = 1;}
    [_dccDecoderObject setCv:SkipEven withValue:newValue];
    [self colorGetButtonSwitchTab:1];
};

- (IBAction)selectedSendFB:(id)sender{
    NSString *newString = [sender stringValue];
    uint8_t newValue = 1; // Select the default value
    if ([newString isEqualToString:@"No"])  {newValue = 0;}
    if ([newString isEqualToString:@"Yes"]) {newValue = 1;}
    [_dccDecoderObject setCv:SendFB withValue:newValue];
    [self colorGetButtonSwitchTab:1];
};

- (IBAction)selectedAlwaysAct:(id)sender{
    NSString *newString = [sender stringValue];
    uint8_t newValue = 1; // Select the default value
    if ([newString isEqualToString:@"No"])  {newValue = 0;}
    if ([newString isEqualToString:@"Yes"]) {newValue = 1;}
    [_dccDecoderObject setCv:AlwaysAct withValue:newValue];
    [self colorGetButtonSwitchTab:1];
};

- (IBAction)pushedSwitchTabDefaults:(id)sender {
  [self readLastInputFromTextFields];
  [_dccDecoderObject setCv:T_on_F1   withValue:15]; // Delay 1
  [_dccDecoderObject setCv:T_on_F2   withValue:15]; // Delay 2
  [_dccDecoderObject setCv:T_on_F3   withValue:15]; // Delay 3
  [_dccDecoderObject setCv:T_on_F4   withValue:15]; // Delay 4
  [_dccDecoderObject setCv:SkipEven  withValue:1];  // Default is ON
  [_dccDecoderObject setCv:SendFB    withValue:1];  // Default is ON
  [_dccDecoderObject setCv:AlwaysAct withValue:1];  // Default is ON
  [self colorGetButtonSwitchTab:1];
  [self updateSwitchTab];
}

- (IBAction)pushedSwitchTabGet:(id)sender {
  [self readLastInputFromTextFields];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:T_on_F1];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:T_on_F2];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:T_on_F3];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:T_on_F4];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:SkipEven];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:SendFB];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:AlwaysAct];
  [_tcpConnectionsObject sendNextPomVerifyPacketFromQueue];
  [self colorGetButtonSwitchTab:0];
}

- (IBAction)pushedSwitchTabSet:(id)sender {
  [self readLastInputFromTextFields];
  [_tcpConnectionsObject queuePomWritePacketForCV:T_on_F1];
  [_tcpConnectionsObject queuePomWritePacketForCV:T_on_F2];
  [_tcpConnectionsObject queuePomWritePacketForCV:T_on_F3];
  [_tcpConnectionsObject queuePomWritePacketForCV:T_on_F4];
  [_tcpConnectionsObject queuePomWritePacketForCV:SkipEven];
  [_tcpConnectionsObject queuePomWritePacketForCV:SendFB];
  [_tcpConnectionsObject queuePomWritePacketForCV:AlwaysAct];
  [_tcpConnectionsObject sendNextPomWritePacketFromQueue];
  [self colorGetButtonSwitchTab:0];
}


- (void) updateSwitchTab {
  NSString *newString;
  [_cv3Text   setFloatValue:([self cvValueFloat:T_on_F1] / 50)]; // in 20 mseconds
  [_cv4Text   setFloatValue:([self cvValueFloat:T_on_F2] / 50)];
  [_cv5Text   setFloatValue:([self cvValueFloat:T_on_F3] / 50)];
  [_cv6Text   setFloatValue:([self cvValueFloat:T_on_F4] / 50)];
  [_cv3Slider setFloatValue:([self cvValueFloat:T_on_F1] / 50)];
  [_cv4Slider setFloatValue:([self cvValueFloat:T_on_F2] / 50)];
  [_cv5Slider setFloatValue:([self cvValueFloat:T_on_F3] / 50)];
  [_cv6Slider setFloatValue:([self cvValueFloat:T_on_F4] / 50)];
  // SkipEven
  newString = @"Yes";      // default
  if ([_dccDecoderObject getCv:SkipEven] == 0) newString = @"No";
  if ([_dccDecoderObject getCv:SkipEven] == 1) newString = @"Yes";
  [_comboSkipEven selectItemWithObjectValue: newString];
  // SendFB
  newString = @"Yes";      // default
  if ([_dccDecoderObject getCv:SendFB] == 0) newString = @"No";
  if ([_dccDecoderObject getCv:SendFB] == 1) newString = @"Yes";
  [_comboSendFB selectItemWithObjectValue: newString];
  // AlwaysAct
  newString = @"Yes";      // default
  if ([_dccDecoderObject getCv:AlwaysAct] == 0) newString = @"No";
  if ([_dccDecoderObject getCv:AlwaysAct] == 1) newString = @"Yes";
  [_comboAlwaysAct selectItemWithObjectValue: newString];
}

- (void)storeSwitchDelayCv:(uint16_t)cvNumber with:(float)cvFloatValue {
  if (cvFloatValue > 2.55) {cvFloatValue = 2.55;}
  if (cvFloatValue < 0.00) {cvFloatValue = 0;}
  uint8_t intValue = cvFloatValue * 50;
  [_dccDecoderObject setCv:cvNumber withValue:intValue];
  [self updateTabs];
  [self colorGetButtonSwitchTab:1];
}

-(float)cvValueFloat:(int)number {
  // Used to transform the CV value (0..255) into a float (0,00 .. 2,55)
  // Note: we have to add a very small margin to cope with rounding errors in the TextField
  float margin = 0.0001;
  float temp = [[NSNumber numberWithInt:[_dccDecoderObject getCv:number]]floatValue];
  return (temp + margin);
}

- (void)colorGetButtonSwitchTab:(int)isRed {
  if (isRed) [self setButtonTitleFor:_buttonSwitchTabSet toString:@"SET" withColor:[NSColor redColor]];
    else     [self setButtonTitleFor:_buttonSwitchTabSet toString:@"SET" withColor:[NSColor blackColor]];
}



// ************************************************************************************************************
// ********************************** USER INTERFACE METHODS - INITIALISE TAB **********************************
// ************************************************************************************************************
- (IBAction)enteredNewAddress:(id)sender {
  // NSLog(@"User input - New switch address: %i", [sender intValue]);
  int newValue = [sender intValue];
  if (newValue < 1) {newValue = 1;}
  if (newValue > 1024) {newValue = 1024;}
  newValue = newValue - 1;   // entering is 1..1024 <-> storing is 0..1023
  newValue = newValue >> 2;   // decoder address = switch address DIV 4
  // Modified September 2020: Change for newer decoder software the value of CV1
  // CV1 should be in the range 0..63.
  // However, according to RCN213, for the first handheld address (switch = 1) CV1 should become 1.
  // In my original decoder software (before 2020, version <= 10) handheld address = 1 resulted in CV1 = 0
  // To accomodate both versions, at startup this program tries to read CV values using loco address 6999
  // If a non-initialised decoder responds, we know the decoder's software version and can use the
  // appropriate algorithm to set CV1
  // NSLog(@"Software version:%d", [_dccDecoderObject getCv:version]);
  if ([_dccDecoderObject getCv:version] > 10) {
    [_dccDecoderObject setCv:myAddrL withValue:( (newValue + 1) & 0b00111111)];
    [_dccDecoderObject setCv:myAddrH withValue:(((newValue + 1) >> 6) & 0b00000111)];
  }
  else {
    [_dccDecoderObject setCv:myAddrL withValue:(newValue & 0b00111111)];
    [_dccDecoderObject setCv:myAddrH withValue:((newValue >> 6) & 0b00000111)];
  }
  // Set the RS-Bus address
  if (newValue <=128) [_dccDecoderObject setCv:myRSAddr withValue:(newValue + 1)];
    else  [_dccDecoderObject setCv:myRSAddr withValue:0];
  [self updateTabs];
  [self colorGetButtonIniTab:1];
}

- (IBAction)pushedIniTabSet:(id)sender {
  [self readLastInputFromTextFields];
  // Set the address of the uninitilised decoder to -1, which will result in a PoM message with address 6999
  [_dccDecoderObject setDecoderAddress:-1];
  [_dccDecoderObject setCv:Restart withValue:1];
  [_address setIntValue:0];
  // Check the values from CV1 & CV9, since wrong values make the decoder unaccessable
  int switchAddr = [_dccDecoderObject getSwitchAddress];
  if ((switchAddr >= 1) && (switchAddr <= 1024)) {
    // NSLog(@"New Switch address: %i", switchAddr);
    [_tcpConnectionsObject queuePomWritePacketForCV:myAddrL];
    [_tcpConnectionsObject queuePomWritePacketForCV:myAddrH];
    [_tcpConnectionsObject queuePomWritePacketForCV:myRSAddr];
    [_tcpConnectionsObject queuePomWritePacketForCV:Restart];
    [_tcpConnectionsObject sendNextPomWritePacketFromQueue];
    // Update the decoder address, but wait one second to allow all PoM messages to be send using the "old" address for non-initialised decoders
    [self performSelector:@selector(updateDecoderAddress) withObject:nil afterDelay:1];
    [self colorGetButtonIniTab:0];
  }
  [self updateTabs];
}

- (void)updateDecoderAddress {
  [_dccDecoderObject setDecoderAddress:[_dccDecoderObject getDecoderAddress]];
  [_address setIntValue:[_dccDecoderObject getSwitchAddress]];
}

- (void)updateIniTab {
  [_addressNew setIntValue:[_dccDecoderObject getSwitchAddress]];
  [_iniTabDecoderAddress setIntValue:[_dccDecoderObject getDecoderAddress]];
  int rsAddress = [_dccDecoderObject getCv:myRSAddr];
  if (rsAddress <=128)[_iniTabRsAddress setIntValue:rsAddress];
  else [_iniTabRsAddress setStringValue:@"None"];
}

- (void)colorGetButtonIniTab:(int)isRed {
  if (isRed) [self setButtonTitleFor:_buttonIniTabSet toString:@"SET" withColor:[NSColor redColor]];
    else     [self setButtonTitleFor:_buttonIniTabSet toString:@"SET" withColor:[NSColor blackColor]];
}


// ************************************************************************************************************
// ************************************ USER INTERFACE METHODS - CV TAB ***************************************
// ************************************************************************************************************
- (IBAction)enteredCvNumber:(id)sender {
  int newValue = [sender intValue];  
  _dccDecoderObject.cvNumber = newValue;
}

- (IBAction)enteredCvValue:(id)sender {
  uint8_t newValue = [sender intValue];
  _dccDecoderObject.cvValue = newValue;
}

- (IBAction)pushedCvTabGet:(id)sender {
  [self readLastInputFromTextFields];
  int cvNumber = [_dccDecoderObject cvNumber];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:cvNumber];
  [_tcpConnectionsObject sendNextPomVerifyPacketFromQueue];
}


- (IBAction)pushedCvTabSet:(id)sender {
  [self readLastInputFromTextFields];
  [_window makeFirstResponder:sender];
  int cvNumber = [_dccDecoderObject cvNumber];
  uint8_t cvValue =  [_dccDecoderObject cvValue];
  if (cvNumber > 0) {
    [_dccDecoderObject setCv:cvNumber withValue:cvValue];
    [_tcpConnectionsObject queuePomWritePacketForCV:cvNumber];
    [_tcpConnectionsObject sendNextPomWritePacketFromQueue];
  }
  [self updateTabs];
}

- (IBAction)pushedCvTabResetCvs:(id)sender {
  [_dccDecoderObject setCv:VID withValue:13];
  [_tcpConnectionsObject queuePomWritePacketForCV:VID];
  [_tcpConnectionsObject sendNextPomWritePacketFromQueue];
  [self updateTabs];
}

- (void)updateCvTab {
  if ((_dccDecoderObject.cvNumber > 0) && (_dccDecoderObject.cvNumber <=512)) 
    [_cvValue setIntValue:[_dccDecoderObject getCv:_dccDecoderObject.cvNumber]];
}


// ************************************************************************************************************
// *************************************** Support function for all buttons ***********************************
// ************************************************************************************************************
- (void)setButtonTitleFor:(NSButton*)button toString:(NSString*)title withColor:(NSColor*)color {
  NSMutableParagraphStyle *style = [[NSMutableParagraphStyle alloc] init];
  [style setAlignment:NSTextAlignmentCenter];
  NSDictionary *attrsDictionary = [NSDictionary dictionaryWithObjectsAndKeys:
                                   color, NSForegroundColorAttributeName, style, NSParagraphStyleAttributeName, nil];
  NSAttributedString *attrString = [[NSAttributedString alloc]
                                    initWithString:title attributes:attrsDictionary];
  [button setAttributedTitle:attrString];
  [style release];
  [attrString release]; 
}

- (void) readLastInputFromTextFields{
  // Trick in which we move focus away from TextFields (and others)
  // This ensures the value in the "current" textfield (thus the one that has focus) gets read
  // Should be called from all IBActions associated with buttons
  [_window makeFirstResponder:nil];
}


// ************************************************************************************************************
// **************************************** CODE FOR HIDING / REMOVING TABS ***********************************
// ************************************************************************************************************
NSTabViewItem *holderForTabSwitch;

- (void)hideSwitchTab{
  if (holderForTabSwitch == nil) {
    holderForTabSwitch = [[_windowTabs tabViewItemAtIndex:3] retain];
    [_windowTabs removeTabViewItem:holderForTabSwitch];
  }
}

- (void)showSwitchTab{
  if (holderForTabSwitch != nil) {
    [_windowTabs insertTabViewItem:holderForTabSwitch atIndex:3];
    holderForTabSwitch = nil;
  }
}


// ************************************************************************************************************
// ***************************************** PREFERENCES WINDOW ***********************************************
// ************************************************************************************************************
-(IBAction)showPreferences:(id)sender { 
  if (_preferencesController == nil)
    _preferencesController = [[PreferencesController alloc] initWithWindowNibName:@"PreferencesController"];
  [_preferencesController showWindow:self]; 
}

- (void)checkPreferences{
  // Test if we can read the Preferences
  NSString *test1 = [[NSUserDefaults standardUserDefaults] objectForKey:@"defaultIpAddressForSending"];
  NSString *test2 = [[NSUserDefaults standardUserDefaults] objectForKey:@"defaultIpAddressForReceiving"];
  NSString *test3 = [[NSUserDefaults standardUserDefaults] objectForKey:@"defaultIpAddressForReceivingIsActive"];
  // Check if all preferences exist
  if ((test1 == nil) || (test2 == nil) || (test3 == nil)) [self initialisePreferences];
}


- (void)initialisePreferences{
  // Create a new preferences file
  [[NSUserDefaults standardUserDefaults] setObject:@"192.168.1.212"        forKey:@"defaultIpAddressForSending"];
  [[NSUserDefaults standardUserDefaults] setObject:@"192.168.1.213"        forKey:@"defaultIpAddressForReceiving"];
  [[NSUserDefaults standardUserDefaults] setBool:1                         forKey:@"defaultIpAddressForReceivingIsActive"];
}

// ************************************************************************************************************
// *************************************** dealloc and closing procedures *************************************
// ************************************************************************************************************
- (void)dealloc {[super dealloc];}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {return YES;}


- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
  [_tcpConnectionsObject closeTcp];
  return NSTerminateNow;
}




@end
