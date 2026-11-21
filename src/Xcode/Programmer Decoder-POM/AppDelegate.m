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
@synthesize addressRange                 = _addressRange;      // 2026
@synthesize buttonSelect               = _buttonSelect;      // 2026
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
@synthesize comboSwitchTabSkipUnEven     = _comboSwitchTabSkipUnEven;   // 2026: was comboSkipEven
@synthesize comboSendFB                  = _comboSendFB;
@synthesize comboAlwaysAct               = _comboAlwaysAct;
@synthesize buttonSwitchTabDefaults    = _buttonSwitchTabDefaults;
@synthesize buttonSwitchTabGet         = _buttonSwitchTabGet;
@synthesize buttonSwitchTabSet         = _buttonSwitchTabSet;
//
// RS-Bus tab properties
@synthesize comboRsFec                 = _comboRsFec;
@synthesize comboRsParity              = _comboRsParity;
@synthesize comboRsPulsCount           = _comboRsPulsCount;
@synthesize parityErrorsText           = _parityErrorsText;
@synthesize pulseErrorsText            = _pulseErrorsText;
@synthesize buttonRsBusTabSet          = _buttonRsBusTabSet;
//
// Servo tabs properties (2026)
@synthesize comboSkipUnEven            = _comboSkipUnEven;
@synthesize comboSwitchCount           = _comboSwitchCount;
@synthesize comboSwitchType            = _comboSwitchType;
@synthesize textServosOnBoard          = _textServosOnBoard;
@synthesize buttonServoTabDefaults     = _buttonServoTabDefaults;
@synthesize buttonServoTabGet          = _buttonServoTabGet;
@synthesize buttonServoTabSet          = _buttonServoTabSet;
@synthesize popupServoPositions        = _popupServoPositions;
@synthesize radioServoMin              = _radioServoMin;
@synthesize radioServoMax              = _radioServoMax;
@synthesize radioServoSpeed            = _radioServoSpeed;
@synthesize sliderServoMin             = _sliderServoMin;
@synthesize sliderServoMax             = _sliderServoMax;
@synthesize sliderServoSpeed           = _sliderServoSpeed;
@synthesize fieldServoMin              = _fieldServoMin;
@synthesize fieldServoMax              = _fieldServoMax;
@synthesize fieldServoSpeed            = _fieldServoSpeed;
@synthesize stepperServoMin            = _stepperServoMin;
@synthesize stepperServoMax            = _stepperServoMax;
@synthesize stepperServoSpeed          = _stepperServoSpeed;
@synthesize textServoStatus            = _textServoStatus;
@synthesize buttonServoStart           = _buttonServoStart;
@synthesize buttonServoMiddle          = _buttonServoMiddle;
@synthesize buttonServoDefaults        = _buttonServoDefaults;
@synthesize buttonServoCancel          = _buttonServoCancel;
@synthesize buttonServoDone            = _buttonServoDone;
@synthesize popupServoCvs              = _popupServoCvs;
@synthesize comboCurveA                = _comboCurveA;
@synthesize checkCurveAReverse         = _checkCurveAReverse;
@synthesize comboCurveB                = _comboCurveB;
@synthesize checkCurveBReverse         = _checkCurveBReverse;
@synthesize comboInvertServoDir        = _comboInvertServoDir;
@synthesize comboInvertRelais          = _comboInvertRelais;
@synthesize comboServoType             = _comboServoType;
@synthesize comboPulseStartUpValue     = _comboPulseStartUpValue;
@synthesize fieldPowerStartUpDelay     = _fieldPowerStartUpDelay;
@synthesize comboIdlePulseDefault      = _comboIdlePulseDefault;
@synthesize fieldPulseOnBefore         = _fieldPulseOnBefore;
@synthesize fieldPulseOffAfter         = _fieldPulseOffAfter;
@synthesize comboPowerWhenIdle         = _comboPowerWhenIdle;
@synthesize fieldPowerOnBefore         = _fieldPowerOnBefore;
@synthesize fieldPowerOffAfter         = _fieldPowerOffAfter;
@synthesize buttonServoCvsTabDefaults  = _buttonServoCvsTabDefaults;
@synthesize buttonServoCvsTabGet       = _buttonServoCvsTabGet;
@synthesize buttonServoCvsTabSet       = _buttonServoCvsTabSet;
//
// Lift tab properties (2026)
@synthesize comboStartHoming           = _comboStartHoming;
@synthesize comboIrDetect              = _comboIrDetect;
@synthesize comboLcdDisplay            = _comboLcdDisplay;
@synthesize comboSerialLine            = _comboSerialLine;
@synthesize buttonLiftTabDefaults      = _buttonLiftTabDefaults;
@synthesize buttonLiftTabGet           = _buttonLiftTabGet;
@synthesize buttonLiftTabSet           = _buttonLiftTabSet;
//
// Initialise tab properties
@synthesize addressNew                   = _addressNew;
@synthesize iniTabDecoderAddress         = _iniTabDecoderAddress;
@synthesize iniTabRsAddress              = _iniTabRsAddress;
@synthesize iniTabAddressRange           = _iniTabAddressRange;   // 2026
@synthesize iniTabExplanation            = _iniTabExplanation;    // 2026
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
#define SkipUnEven    21    // 2026: was SkipEven. Name as in AP_DCC_Decoder_Core
#define RSParity      22    // 2026
#define Search        23
#define RSPulsCount   24    // 2026
#define Restart       25
#define DccQuality    26
#define DecType       27
#define BiDi          28
#define Config        29
#define VID_2         30
#define ParityErrors  31    // 2026: read-only
#define PulseErrors   32    // 2026: read-only
#define SendFB        33
#define AlwaysAct     34
// Lift decoder (2026)
#define StartHoming   33
#define IR_Detect     34
#define LCD_Display   35
#define Serial_Line   36
// Servo decoders (2026). Names as in AP_DCC_Decoder_Core and AVR-Servo-3-6 (servo_CVs.h, configure_pom.h)
#define SwitchCount              35
#define SwitchType               36
#define CV_POM_CONFIG_MODE       62    // RAM only: what should be configured (NoMode, MinMode, MaxMode, SpeedMode)
#define CV_POM_CONFIG_SERVO      63    // RAM only: which servo should be configured (0 = none, 1..CV64)
#define EEPROM_MAX_SERVOS_INDEX  64    // Maximum number of servos of the board (read only)
#define START_INDEX_SERVO_CVS    65
#define NUMBER_OF_SERVO_CVS      18
// Offsets of the CVs of each servo
#define MinLow                    0
#define MinHigh                   1
#define MaxLow                    2
#define MaxHigh                   3
#define CurveA                    4
#define CurveB                    5
#define Speed                     6
#define InvertServoDir            7
#define InvertRelais              8    // Only used by the servo-2 board (relays)
#define ServoType                 9
#define PulseStartUpValue        10
#define PowerStartUpDelay        11
#define IdlePulseDefault         12
#define PulseOnBefore            13
#define PulseOffAfter            14
#define PowerWhenIdle            15
#define PowerOnBefore            16
#define PowerOffAfter            17
// Values of SwitchType
#define NORMAL                    0
#define DKW                       1
#define HOSENTRAGER               2
// Values of CV_POM_CONFIG_MODE
#define NoMode                    0
#define MinMode                   1
#define MaxMode                   2
#define SpeedMode                 3

// 2026: State of the servo tabs
int selectedServo = 1;              // 1..6: servo shown on the SERVO POSITIONS and SERVO CVS tabs
int liveServo = 0;                  // 0: no live setting. 1..6: servo being set live (CV63)
int liveMode = MinMode;             // MinMode, MaxMode or SpeedMode (CV62), as selected by the radio buttons
uint16_t originalMin;               // Values read before START, used by CANCEL
uint16_t originalMax;
uint8_t originalSpeed;
BOOL endLiveAfterWrites = NO;       // CANCEL: end the live setting once the original values are written
BOOL terminateAfterWrites = NO;     // Quit during a live setting: terminate once the live setting is ended
// 2026: Switch addresses and non-initialised decoders
int maxSwitchAddress = 1024;        // 2048 if the software version of the command station is 3.8 or higher
BOOL nonInitialisedFound = NO;      // A non-initialised decoder (address -1, loco address 6999) has answered
NSString *curveNames[12] = {@"lin_A", @"lin_B", @"move_A", @"move_B", @"sine_A", @"sine_B",
                            @"whip_A", @"whip_B", @"sig_hp0", @"sig_hp1", @"hp1p", @"sine_AB"};
NSString *servoTypeNames[5] = {@"Generic", @"Uhlenbrock 81420 / Weinert", @"MBTronic", @"SG90 Tower Pro", @"SG90 TZT"};


// ************************************************************************************************************
// ********************************************** INITIALIZATION **********************************************
// ************************************************************************************************************
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
  // 2026: The AppDelegate decides if another tab may be selected (see tabView:shouldSelectTabViewItem:)
  [_windowTabs setDelegate:self];
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
  // 2026: The same search is done if SELECT is pushed while the address field is empty
  [self searchNonInitialisedDecoder];
}

- (void)searchNonInitialisedDecoder {
  // 2026: Before, this code was part of applicationDidFinishLaunching (step 6)
  [_dccDecoderObject initialise];   // 2026: Forget the CVs of the previous decoder
  nonInitialisedFound = NO;
  selectedServo = 1;
  [_dccDecoderObject setDecoderAddress:-1];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:version];
  [_tcpConnectionsObject sendNextPomVerifyPacketFromQueue];
  // But in addition we read all other CVs to allow the appropriate TABs (switch, servo, ...) to be displayed.
  [self readAllCvs];
  [self updateTabs];
}

- (void)setMaxSwitchAddress:(int)maxAddress {
  // 2026: Called by the TCP object once the software version of the command station is known
  // Command stations with software version 3.8 or higher support 2048 switch addresses, older ones 1024
  NSString *oldRange = [NSString stringWithFormat:@"1...%d", maxSwitchAddress];
  NSString *newRange = [NSString stringWithFormat:@"1...%d", maxAddress];
  NSString *oldLast  = [NSString stringWithFormat:@"... %d.", maxSwitchAddress - 3];
  NSString *newLast  = [NSString stringWithFormat:@"... %d.", maxAddress - 3];
  NSString *text = [_iniTabExplanation stringValue];
  text = [text stringByReplacingOccurrencesOfString:oldRange withString:newRange];
  text = [text stringByReplacingOccurrencesOfString:oldLast withString:newLast];
  [_iniTabExplanation setStringValue:text];
  maxSwitchAddress = maxAddress;
  [(NSNumberFormatter *)[_address formatter] setMaximum:[NSNumber numberWithInt:maxAddress]];
  [(NSNumberFormatter *)[_addressNew formatter] setMaximum:[NSNumber numberWithInt:maxAddress]];
  [_addressRange setStringValue:[NSString stringWithFormat:@"1..%d", maxAddress]];
  [_iniTabAddressRange setStringValue:[NSString stringWithFormat:@"1..%d", maxAddress]];
}

- (void) updateTabs {
  [self updateMainWindow];
  [self updateControlTab];
  [self updateRsBusTab];
  [self updateSwitchTab];
  [self updateLiftTab];
  [self updateServoTab];
  [self updateServoPositionsTab];
  [self updateServoCvsTab];
  [self updateIniTab];
  [self updateCvTab];
  // Depending on decoder type, show the SWITCH, LIFT or the three SERVO tabs
  // 2026: First hide, then show. The decoder specific tabs start at index 3
  uint8_t decType = [_dccDecoderObject getCv:DecType];
  BOOL isServo = ((decType == 20) || (decType == 21) || (decType == 22));
  if (decType != 16) [self hideSwitchTab];
  if (decType != 24) [self hideLiftTab];
  if (!isServo) {[self hideServoTab]; [self hideServoPositionsTab]; [self hideServoCvsTab];}
  if (decType == 16) [self showSwitchTab];
  if (decType == 24) [self showLiftTab];
  if (isServo) {[self showServoTab]; [self showServoPositionsTab]; [self showServoCvsTab];}
}

- (void)queueTypeSpecificCvs {
  // 2026: Called once the decoder type (CV27) is received. Only the CVs shown on the decoder specific tab are queued.
  // Since the verify queue is processed in ascending CV order, these CVs (33 and higher) are read in the same run.
  uint8_t decType = [_dccDecoderObject getCv:DecType];
  if (decType == 16) {   // SWITCH tab
    [_tcpConnectionsObject queuePomVerifyPacketForCV:SendFB];
    [_tcpConnectionsObject queuePomVerifyPacketForCV:AlwaysAct];
  }
  if (decType == 24) {   // LIFT tab
    [_tcpConnectionsObject queuePomVerifyPacketForCV:StartHoming];
    [_tcpConnectionsObject queuePomVerifyPacketForCV:IR_Detect];
    [_tcpConnectionsObject queuePomVerifyPacketForCV:LCD_Display];
    [_tcpConnectionsObject queuePomVerifyPacketForCV:Serial_Line];
  }
  if ((decType == 20) || (decType == 21) || (decType == 22)) {   // SERVO tabs
    [_tcpConnectionsObject queuePomVerifyPacketForCV:SwitchCount];
    [_tcpConnectionsObject queuePomVerifyPacketForCV:SwitchType];
    [_tcpConnectionsObject queuePomVerifyPacketForCV:EEPROM_MAX_SERVOS_INDEX];
    for (int offset = 0; offset < NUMBER_OF_SERVO_CVS; offset++)
      [_tcpConnectionsObject queuePomVerifyPacketForCV:[self servoCv:offset]];
  }
}


// ************************************************************************************************************
// ********************************** FEEDBACK MESSAGE RECEIVED BY THE TCP OBJECT *****************************
// ************************************************************************************************************
- (void)feedbackPacketReceivedForAddress:(int)rsBusAddress withCV:(int)cvNumber withValue:(u_int8_t)cvValue {
  // NSLog(@"Feedback received. Address:%d CV:%d Value:%d", rsBusAddress, cvNumber, cvValue);
  [_dccDecoderObject setCv:cvNumber withValue:cvValue];
  // 2026: A non-initialised decoder has answered. Report this and show the INITIALISE tab
  if ((cvNumber == version) && ([_dccDecoderObject decoderAddress] == -1) && (!nonInitialisedFound)) {
    nonInitialisedFound = YES;
    [self showNonInitialisedMessage];
    [_windowTabs selectTabViewItemWithIdentifier:@"7"];
  }
  // 2026: Once the decoder type is known, also read the CVs specific for that type of decoder
  // The TCP object sends the next verify packet after this method returns
  if (cvNumber == DecType) [self queueTypeSpecificCvs];
  [self updateTabs];
}

- (void)sendNextPomVerifyPacketFromQueueCompleted{
  // NSLog(@"All PoM verify packets are send");
  // 2026: The TCP object clears the general status line before calling this method. Show the message again
  if (nonInitialisedFound && ([_dccDecoderObject decoderAddress] == -1)) [self showNonInitialisedMessage];
}

- (void)showNonInitialisedMessage {
  [self showGeneralStatus:[NSString stringWithFormat:@"Non-initialised decoder found (software version %d)",
                           [_dccDecoderObject getCv:version]]];
}

- (void)sendNextPomWritePacketFromQueueCompleted{
  // NSLog(@"All PoM write packets are send");
  // 2026: This method is called before the TCP object returns to the INACTIVE state and clears its write queue.
  // New writes are therefore started after this method has returned (afterDelay:0).
  if (endLiveAfterWrites)   [self performSelector:@selector(endLiveSettingAfterCancel) withObject:nil afterDelay:0.0];
  if (terminateAfterWrites) [self performSelector:@selector(terminateNow) withObject:nil afterDelay:0.0];
}

- (void)sendQueuedWrites {
  // 2026: Starts sending the queued PoM writes. The TCP object discards the queued writes if sending is started
  // while a verify sequence (or Lenz status check) is running. In that case we try again a little later.
  // If a write sequence is running, that sequence also sends the newly queued CVs; starting it again would
  // send two messages at the same time.
  operationalState_t state = [_tcpConnectionsObject operationalState];
  if (state == INACTIVE) [_tcpConnectionsObject sendNextPomWritePacketFromQueue];
  else if (state != POM_WRITE) [self performSelector:@selector(sendQueuedWrites) withObject:nil afterDelay:0.1];
}

// ************************************************************************************************************
// ************************************* USER INTERFACE METHODS - MAIN WINDOW *********************************
// ************************************************************************************************************
- (IBAction)takeDecoderAddressFrom:(id)sender {
  // NSLog(@"User input - Decoder address: %i", [sender intValue]);
  // 2026: An empty field means: search for a non-initialised decoder (see selectAddressPushed)
  if ([[sender stringValue] length] == 0) {[_dccDecoderObject setDecoderAddress:-1]; return;}
  int newValue = [sender intValue];
  if (newValue < 1) {newValue = 1;}
  if (newValue > maxSwitchAddress) {newValue = maxSwitchAddress;}   // 2026: was 1024
  newValue = newValue - 1;   // entering is 1..1024 (or 1..2048) <-> storing is 0..1023 (or 0..2047)
  newValue = newValue >> 2;   // decoder address = switch address DIV 4
  if (newValue != [_dccDecoderObject decoderAddress]) selectedServo = 1;   // 2026: other decoder: start with servo 1
  [_dccDecoderObject setDecoderAddress:newValue];
  nonInitialisedFound = NO;
  [_tcpConnectionsObject openTcpIfClosed];   // 2026: reconnect if the Lenz interface(s) were not available before
}

- (IBAction)selectAddressPushed:(id)sender {
  [self readLastInputFromTextFields];
  [_tcpConnectionsObject openTcpIfClosed];   // 2026: reconnect if the Lenz interface(s) were not available before
  // 2026: With an empty address field we search again for a non-initialised decoder
  if ([[_address stringValue] length] == 0) [self searchNonInitialisedDecoder];
  else [self readAllCvs];
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
  // 2026: A restart also ends a live setting of a servo, since CV62 and CV63 only exist in RAM
  endLiveAfterWrites = NO;
  if (liveServo != 0) [self liveSettingEnded];
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
  for (cvNumber=19; cvNumber <= 32; cvNumber++) [_tcpConnectionsObject queuePomVerifyPacketForCV:cvNumber]; // 2026: incl. CV22, 31, 32
  // 2026: The type specific CVs (33 and higher) are queued once the decoder type (CV27) is received
  [_tcpConnectionsObject sendNextPomVerifyPacketFromQueue];
  // Remove possible red color from SET buttons
  [self colorGetButtonControlTab:0];
  [self colorGetButtonRsBusTab:0];
  [self colorGetButtonSwitchTab:0];
  [self colorGetButtonLiftTab:0];
  [self colorGetButtonServoTab:0];
  [self colorGetButtonServoCvsTab:0];
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
  [_comboRsFec setIntValue:newValue];   // 2026: RSRetry is also shown on the RS-Bus tab
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
  if ([newString isEqualToString:@"servo-6"])     {newValue = 22;}  // 2026
  if ([newString isEqualToString:@"lift"])        {newValue = 24;}  // 2026
  if ([newString isEqualToString:@"relays-4"])    {newValue = 32;}
  if ([newString isEqualToString:@"relays-16"])   {newValue = 33;}
  if ([newString isEqualToString:@"function"])    {newValue = 64;}  // 2026
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
  if ([_dccDecoderObject getCv:DecType] == 22)  newString = @"servo-6";   // 2026
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
  [_comboRsRetry setIntValue:newValue];   // 2026: RSRetry is also shown on the Control tab
  [self colorGetButtonRsBusTab:1];
}

- (IBAction)selectedRsParity:(id)sender {
  uint8_t newValue = [sender intValue];
  if (newValue < 0) {newValue = 0;}
  if (newValue > 2) {newValue = 2;}
  [_dccDecoderObject setCv:RSParity withValue:newValue];
  [self colorGetButtonRsBusTab:1];
}

- (IBAction)selectedRsPulsCount:(id)sender {
  uint8_t newValue = [sender intValue];
  if (newValue < 0) {newValue = 0;}
  if (newValue > 2) {newValue = 2;}
  [_dccDecoderObject setCv:RSPulsCount withValue:newValue];
  [self colorGetButtonRsBusTab:1];
}

- (IBAction)pushedRsBusTabDefaults:(id)sender {
  // Default values are taken from CvValues.cpp (AP_DCC_Decoder_Core)
  [self readLastInputFromTextFields];
  [_dccDecoderObject setCv:RSRetry     withValue:0];  // No retransmissions
  [_dccDecoderObject setCv:RSParity    withValue:1];  // Retransmit if data was just transmitted
  [_dccDecoderObject setCv:RSPulsCount withValue:2];  // Always retransmit
  [self colorGetButtonRsBusTab:1];
  [self updateTabs];
}

- (IBAction)pushedRsBusTabGet:(id)sender {
  [self readLastInputFromTextFields];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:RSRetry];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:RSParity];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:RSPulsCount];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:ParityErrors];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:PulseErrors];
  [_tcpConnectionsObject sendNextPomVerifyPacketFromQueue];
  [self colorGetButtonRsBusTab:0];
}

- (IBAction)pushedRsBusTabSet:(id)sender {
  // ParityErrors and PulseErrors are read-only, and are therefore not written
  [self readLastInputFromTextFields];
  [_tcpConnectionsObject queuePomWritePacketForCV:RSRetry];
  [_tcpConnectionsObject queuePomWritePacketForCV:RSParity];
  [_tcpConnectionsObject queuePomWritePacketForCV:RSPulsCount];
  [_tcpConnectionsObject sendNextPomWritePacketFromQueue];
  [self colorGetButtonRsBusTab:0];
}

- (void) updateRsBusTab {
  // RSRetry (same as RSFEC)
  [_comboRsFec setIntValue:[_dccDecoderObject getCv:RSRetry]];
  // 2026: RSParity, RSPulsCount and the (read-only) error counters
  [_comboRsParity    setIntValue:[_dccDecoderObject getCv:RSParity]];
  [_comboRsPulsCount setIntValue:[_dccDecoderObject getCv:RSPulsCount]];
  [_parityErrorsText setIntValue:[_dccDecoderObject getCv:ParityErrors]];
  [_pulseErrorsText  setIntValue:[_dccDecoderObject getCv:PulseErrors]];
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

- (IBAction)selectedSwitchTabSkipUnEven:(id)sender {   // 2026: was selectedSkipEven
    NSString *newString = [sender stringValue];
    uint8_t newValue = 1; // Select the default value
    if ([newString isEqualToString:@"No"])  {newValue = 0;}
    if ([newString isEqualToString:@"Yes"]) {newValue = 1;}
    [_dccDecoderObject setCv:SkipUnEven withValue:newValue];
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
  [_dccDecoderObject setCv:SkipUnEven  withValue:1];  // Default is ON
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
  [_tcpConnectionsObject queuePomVerifyPacketForCV:SkipUnEven];
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
  [_tcpConnectionsObject queuePomWritePacketForCV:SkipUnEven];
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
  // SkipUnEven
  newString = @"Yes";      // default
  if ([_dccDecoderObject getCv:SkipUnEven] == 0) newString = @"No";
  if ([_dccDecoderObject getCv:SkipUnEven] == 1) newString = @"Yes";
  [_comboSwitchTabSkipUnEven selectItemWithObjectValue: newString];
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
// ************************************* USER INTERFACE METHODS - LIFT TAB ************************************
// ************************************************************************************************************
// 2026: CV33..36 of the lift decoder. Values and defaults are taken from CvValues.h / CvValues.cpp (AP_DCC_Decoder_Core)
- (IBAction)selectedStartHoming:(id)sender {[self storeYesNoCv:StartHoming from:sender];}
- (IBAction)selectedIrDetect:(id)sender    {[self storeYesNoCv:IR_Detect   from:sender];}
- (IBAction)selectedLcdDisplay:(id)sender  {[self storeYesNoCv:LCD_Display from:sender];}

- (IBAction)selectedSerialLine:(id)sender {
  NSString *newString = [sender stringValue];
  uint8_t newValue = 0; // Select the default value
  if ([newString isEqualToString:@"Off"])     {newValue = 0;}
  if ([newString isEqualToString:@"On"])      {newValue = 1;}
  if ([newString isEqualToString:@"Details"]) {newValue = 2;}
  [_dccDecoderObject setCv:Serial_Line withValue:newValue];
  [self colorGetButtonLiftTab:1];
}

- (IBAction)pushedLiftTabDefaults:(id)sender {
  [self readLastInputFromTextFields];
  [_dccDecoderObject setCv:StartHoming withValue:1];  // Default is ON
  [_dccDecoderObject setCv:IR_Detect   withValue:1];  // Default is ON
  [_dccDecoderObject setCv:LCD_Display withValue:0];  // Default is OFF
  [_dccDecoderObject setCv:Serial_Line withValue:0];  // Default is OFF
  [self colorGetButtonLiftTab:1];
  [self updateLiftTab];
}

- (IBAction)pushedLiftTabGet:(id)sender {
  [self readLastInputFromTextFields];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:StartHoming];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:IR_Detect];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:LCD_Display];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:Serial_Line];
  [_tcpConnectionsObject sendNextPomVerifyPacketFromQueue];
  [self colorGetButtonLiftTab:0];
}

- (IBAction)pushedLiftTabSet:(id)sender {
  [self readLastInputFromTextFields];
  [_tcpConnectionsObject queuePomWritePacketForCV:StartHoming];
  [_tcpConnectionsObject queuePomWritePacketForCV:IR_Detect];
  [_tcpConnectionsObject queuePomWritePacketForCV:LCD_Display];
  [_tcpConnectionsObject queuePomWritePacketForCV:Serial_Line];
  [_tcpConnectionsObject sendNextPomWritePacketFromQueue];
  [self colorGetButtonLiftTab:0];
}

- (void) updateLiftTab {
  NSString *newString;
  [_comboStartHoming selectItemWithObjectValue:[self yesNoStringForCv:StartHoming]];
  [_comboIrDetect    selectItemWithObjectValue:[self yesNoStringForCv:IR_Detect]];
  [_comboLcdDisplay  selectItemWithObjectValue:[self yesNoStringForCv:LCD_Display]];
  // Serial_Line
  newString = @"Off";      // default
  if ([_dccDecoderObject getCv:Serial_Line] == 1) newString = @"On";
  if ([_dccDecoderObject getCv:Serial_Line] == 2) newString = @"Details";
  [_comboSerialLine selectItemWithObjectValue: newString];
}

- (void)storeYesNoCv:(uint16_t)cvNumber from:(id)sender {
  NSString *newString = [sender stringValue];
  uint8_t newValue = 0;
  if ([newString isEqualToString:@"No"])  {newValue = 0;}
  if ([newString isEqualToString:@"Yes"]) {newValue = 1;}
  [_dccDecoderObject setCv:cvNumber withValue:newValue];
  [self colorGetButtonLiftTab:1];
}

- (NSString *)yesNoStringForCv:(uint16_t)cvNumber {
  if ([_dccDecoderObject getCv:cvNumber] == 0) return @"No";
  return @"Yes";
}

- (void)colorGetButtonLiftTab:(int)isRed {
  if (isRed) [self setButtonTitleFor:_buttonLiftTabSet toString:@"SET" withColor:[NSColor redColor]];
    else     [self setButtonTitleFor:_buttonLiftTabSet toString:@"SET" withColor:[NSColor blackColor]];
}



// ************************************************************************************************************
// ************************************ USER INTERFACE METHODS - SERVO TABS ***********************************
// ************************************************************************************************************
// 2026: Three tabs for the servo decoders (DecType = 20, 21 and 22). The firmware (AVR-Servo-3-6) is leading.
// - SERVO:           the general decoder CVs (CV21, CV35, CV36 and CV64)
// - SERVO POSITIONS: live setting of Min, Max and Speed of one servo, via CV62 and CV63
// - SERVO CVS:       the other CVs of one servo
// CV number of a servo CV = 65 + 18 x (servo - 1) + offset

// ****************************************** Support methods ***********************************************
- (int)servoCv:(int)offset {return START_INDEX_SERVO_CVS + NUMBER_OF_SERVO_CVS * (selectedServo - 1) + offset;}

- (uint16_t)getServoValue16:(int)offset {
  // Min and Max are 16 bit values: low byte at offset, high byte at offset + 1
  return [_dccDecoderObject getCv:[self servoCv:offset]] + 256 * [_dccDecoderObject getCv:[self servoCv:offset + 1]];
}

- (void)setServoValue16:(int)offset to:(uint16_t)value {
  [_dccDecoderObject setCv:[self servoCv:offset]     withValue:(value & 0xFF)];
  [_dccDecoderObject setCv:[self servoCv:offset + 1] withValue:(value >> 8)];
}

- (int)servosOnBoard {
  // CV64 holds the maximum number of servos of the board. If not (yet) available, use the decoder type
  int number = [_dccDecoderObject getCv:EEPROM_MAX_SERVOS_INDEX];
  if ((number >= 1) && (number <= 6)) return number;
  if ([_dccDecoderObject getCv:DecType] == 20) return 2;
  if ([_dccDecoderObject getCv:DecType] == 21) return 3;
  return 6;
}

- (int)switchType {
  // The SwitchType as used by the firmware. The servo-2 board has no SwitchType.
  // Hosentrager requires 4 servos; on a board with less servos the decoder operates as a normal decoder
  int type = [_dccDecoderObject getCv:SwitchType];
  if ([_dccDecoderObject getCv:DecType] == 20) return NORMAL;
  if ((type == HOSENTRAGER) && ([self servosOnBoard] < 4)) return NORMAL;
  if (type > HOSENTRAGER) return NORMAL;
  return type;
}

- (int)servosInUse {
  // DKW uses 3 servos and Hosentrager 4. Otherwise SwitchCount: 0, 255 or above the maximum = board maximum
  int maxServos = [self servosOnBoard];
  int count = [_dccDecoderObject getCv:SwitchCount];
  if ([self switchType] == DKW) return 3;
  if ([self switchType] == HOSENTRAGER) return 4;
  if ((count == 0) || (count == 255) || (count > maxServos)) count = maxServos;
  return count;
}

- (void)fillComboBox:(NSComboBox *)combo with:(NSArray *)items {
  // Only refill if the items differ, so an open list is not disturbed by every feedback message
  if ([[combo objectValues] isEqualToArray:items]) return;
  [combo removeAllItems];
  [combo addItemsWithObjectValues:items];
}

- (void)fillServoPopup:(NSPopUpButton *)popup {
  // The popups of the SERVO POSITIONS and SERVO CVS tabs show the same servo.
  // During a live setting another servo can not be chosen; first DONE or CANCEL.
  int number = [self servosInUse];
  if ([popup numberOfItems] != number) {
    [popup removeAllItems];
    for (int i = 1; i <= number; i++) [popup addItemWithTitle:[NSString stringWithFormat:@"Servo %d", i]];
  }
  [popup selectItemAtIndex:(selectedServo - 1)];
  [popup setEnabled:(liveServo == 0)];
}

- (void)updateServoPopups {
  if ((selectedServo > [self servosInUse]) && (liveServo == 0)) selectedServo = 1;
  [self fillServoPopup:_popupServoPositions];
  [self fillServoPopup:_popupServoCvs];
}

- (IBAction)selectedServo:(id)sender {
  // Used by the popups of the SERVO POSITIONS and SERVO CVS tabs. Read all CVs of the chosen servo
  selectedServo = (int)[sender indexOfSelectedItem] + 1;
  [self readServoCvs];
  [self colorGetButtonServoCvsTab:0];
  [self updateTabs];
}

- (void)readServoCvs {
  for (int offset = 0; offset < NUMBER_OF_SERVO_CVS; offset++)
    [_tcpConnectionsObject queuePomVerifyPacketForCV:[self servoCv:offset]];
  [_tcpConnectionsObject sendNextPomVerifyPacketFromQueue];
}


// ********************************************** SERVO tab *************************************************
- (IBAction)selectedServoTabValue:(id)sender {
  NSString *newString = [sender stringValue];
  uint8_t newValue = 0;
  if (sender == _comboSkipUnEven) {
    if ([newString isEqualToString:@"Yes"]) newValue = 1;
    [_dccDecoderObject setCv:SkipUnEven withValue:newValue];
  }
  if (sender == _comboSwitchCount) {
    if (![newString isEqualToString:@"Maximum"]) newValue = [newString intValue];   // 0 = Maximum
    [_dccDecoderObject setCv:SwitchCount withValue:newValue];
  }
  if (sender == _comboSwitchType) {
    if ([newString isEqualToString:@"DKW"])         newValue = DKW;
    if ([newString isEqualToString:@"Hosentrager"]) newValue = HOSENTRAGER;
    [_dccDecoderObject setCv:SwitchType withValue:newValue];
  }
  [self colorGetButtonServoTab:1];
  [self updateTabs];      // SwitchCount and SwitchType determine the number of servos in the popups
}

- (IBAction)pushedServoTabDefaults:(id)sender {
  [self readLastInputFromTextFields];
  [_dccDecoderObject setCv:SkipUnEven  withValue:1];         // Default for servo decoders (CvValues.cpp)
  [_dccDecoderObject setCv:SwitchCount withValue:0];         // Board maximum
  [_dccDecoderObject setCv:SwitchType  withValue:NORMAL];
  [self colorGetButtonServoTab:1];
  [self updateTabs];
}

- (IBAction)pushedServoTabGet:(id)sender {
  [self readLastInputFromTextFields];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:SkipUnEven];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:SwitchCount];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:SwitchType];
  [_tcpConnectionsObject queuePomVerifyPacketForCV:EEPROM_MAX_SERVOS_INDEX];
  [_tcpConnectionsObject sendNextPomVerifyPacketFromQueue];
  [self colorGetButtonServoTab:0];
}

- (IBAction)pushedServoTabSet:(id)sender {
  // CV64 is read only. The servo-2 board has no SwitchType
  [self readLastInputFromTextFields];
  [_tcpConnectionsObject queuePomWritePacketForCV:SkipUnEven];
  [_tcpConnectionsObject queuePomWritePacketForCV:SwitchCount];
  if ([_dccDecoderObject getCv:DecType] != 20) [_tcpConnectionsObject queuePomWritePacketForCV:SwitchType];
  [self sendQueuedWrites];
  [self colorGetButtonServoTab:0];
}

- (void)updateServoTab {
  int maxServos = [self servosOnBoard];
  int count = [_dccDecoderObject getCv:SwitchCount];
  NSMutableArray *items = [NSMutableArray array];
  NSString *newString;
  // SkipUnEven
  [_comboSkipUnEven selectItemWithObjectValue:[self yesNoStringForCv:SkipUnEven]];
  // SwitchType: DKW requires 3 servos, Hosentrager 4 (servo-6 board only). The servo-2 board has no SwitchType
  [items addObject:@"Normal"];
  if (maxServos >= 3) [items addObject:@"DKW"];
  if (maxServos >= 4) [items addObject:@"Hosentrager"];
  [self fillComboBox:_comboSwitchType with:items];
  newString = @"Normal";
  if ([self switchType] == DKW)         newString = @"DKW";
  if ([self switchType] == HOSENTRAGER) newString = @"Hosentrager";
  [_comboSwitchType selectItemWithObjectValue:newString];
  [_comboSwitchType setEnabled:([_dccDecoderObject getCv:DecType] != 20)];
  // SwitchCount: only used if SwitchType is Normal. For DKW and Hosentrager the firmware determines the number
  items = [NSMutableArray arrayWithObject:@"Maximum"];
  for (int i = 1; i <= maxServos; i++) [items addObject:[NSString stringWithFormat:@"%d", i]];
  [self fillComboBox:_comboSwitchCount with:items];
  newString = @"Maximum";
  if ((count >= 1) && (count <= maxServos)) newString = [NSString stringWithFormat:@"%d", count];
  [_comboSwitchCount selectItemWithObjectValue:newString];
  [_comboSwitchCount setEnabled:([self switchType] == NORMAL)];
  // CV64
  [_textServosOnBoard setIntValue:maxServos];
}

- (void)colorGetButtonServoTab:(int)isRed {
  if (isRed) [self setButtonTitleFor:_buttonServoTabSet toString:@"SET" withColor:[NSColor redColor]];
    else     [self setButtonTitleFor:_buttonServoTabSet toString:@"SET" withColor:[NSColor blackColor]];
}


// ***************************************** SERVO POSITIONS tab ********************************************
// START writes CV63 (servo) and CV62 (mode). Each change of Min, Max or Speed is written immediately;
// the firmware uses a new Min or Max after its high byte. The write queue sends the CVs in ascending order,
// thus the low byte is always written before the high byte. DONE writes CV62 = 0 and CV63 = 0.
// CANCEL first writes the values read before START, and then CV62 = 0 and CV63 = 0.
- (IBAction)selectedServoSetting:(id)sender {
  if (sender == _radioServoMin)   liveMode = MinMode;
  if (sender == _radioServoMax)   liveMode = MaxMode;
  if (sender == _radioServoSpeed) liveMode = SpeedMode;
  if (liveServo != 0) {
    [_dccDecoderObject setCv:CV_POM_CONFIG_MODE withValue:liveMode];
    [_tcpConnectionsObject queuePomWritePacketForCV:CV_POM_CONFIG_MODE];
    [self sendQueuedWrites];
  }
  [self updateServoPositionsTab];
}

- (IBAction)servoSliderMoved:(id)sender    {[self liveValueFrom:sender];}
- (IBAction)servoFieldEntered:(id)sender   {[self liveValueFrom:sender];}
- (IBAction)servoStepperClicked:(id)sender {[self liveValueFrom:sender];}

- (void)liveValueFrom:(id)sender {
  // A change of Min, Max or Speed also selects that mode, so the servo shows the new value
  int mode = SpeedMode;
  if ((sender == _sliderServoMin) || (sender == _fieldServoMin) || (sender == _stepperServoMin)) mode = MinMode;
  if ((sender == _sliderServoMax) || (sender == _fieldServoMax) || (sender == _stepperServoMax)) mode = MaxMode;
  if (liveServo == 0) return;
  if (mode != liveMode) {
    liveMode = mode;
    [_dccDecoderObject setCv:CV_POM_CONFIG_MODE withValue:liveMode];
    [_tcpConnectionsObject queuePomWritePacketForCV:CV_POM_CONFIG_MODE];
  }
  [self storeLiveValue:[sender intValue] forMode:mode];
  [self sendQueuedWrites];
  [self updateServoPositionsTab];
}

- (void)storeLiveValue:(int)value forMode:(int)mode {
  // Min and Max: 544..2400 us (limits of the Servo-TCA library). Speed: 1..50
  int offset = MinLow;
  if (mode == SpeedMode) {
    if (value < 1)  value = 1;
    if (value > 50) value = 50;      // The decoder accepts 1..255; in practice 50 is more than enough
    [_dccDecoderObject setCv:[self servoCv:Speed] withValue:value];
    [_tcpConnectionsObject queuePomWritePacketForCV:[self servoCv:Speed]];
    return;
  }
  if (mode == MaxMode) offset = MaxLow;
  if (value < 544)  value = 544;
  if (value > 2400) value = 2400;
  [self setServoValue16:offset to:value];
  [_tcpConnectionsObject queuePomWritePacketForCV:[self servoCv:offset]];       // low byte
  [_tcpConnectionsObject queuePomWritePacketForCV:[self servoCv:offset + 1]];   // high byte
}

- (IBAction)pushedServoStart:(id)sender {
  [self readLastInputFromTextFields];
  if (liveServo != 0) return;
  liveServo = selectedServo;
  // The values of this servo were read after choosing the servo (or after SELECT). Keep them for CANCEL
  originalMin   = [self getServoValue16:MinLow];
  originalMax   = [self getServoValue16:MaxLow];
  originalSpeed = [_dccDecoderObject getCv:[self servoCv:Speed]];
  [_dccDecoderObject setCv:CV_POM_CONFIG_MODE  withValue:liveMode];
  [_dccDecoderObject setCv:CV_POM_CONFIG_SERVO withValue:liveServo];
  [_tcpConnectionsObject queuePomWritePacketForCV:CV_POM_CONFIG_MODE];
  [_tcpConnectionsObject queuePomWritePacketForCV:CV_POM_CONFIG_SERVO];
  [self sendQueuedWrites];
  // Another decoder can only be selected after DONE or CANCEL
  [_address setEnabled:NO];
  [_buttonSelect setEnabled:NO];
  [self updateTabs];
}

- (IBAction)pushedServoMiddle:(id)sender {
  [self readLastInputFromTextFields];
  if ((liveServo == 0) || (liveMode == SpeedMode)) return;
  [self storeLiveValue:1500 forMode:liveMode];
  [self sendQueuedWrites];
  [self updateServoPositionsTab];
}

- (IBAction)pushedServoDefaults:(id)sender {
  // Defaults as in servo_CVs.cpp. The mode (CV62) does not change
  [self readLastInputFromTextFields];
  if (liveServo == 0) return;
  [self storeLiveValue:1300 forMode:MinMode];
  [self storeLiveValue:1700 forMode:MaxMode];
  [self storeLiveValue:6    forMode:SpeedMode];
  [self sendQueuedWrites];
  [self updateServoPositionsTab];
}

- (IBAction)pushedServoCancel:(id)sender {
  // The popups are disabled during a live setting, thus selectedServo == liveServo
  [self readLastInputFromTextFields];
  if ((liveServo == 0) || endLiveAfterWrites) return;
  [self storeLiveValue:originalMin   forMode:MinMode];
  [self storeLiveValue:originalMax   forMode:MaxMode];
  [self storeLiveValue:originalSpeed forMode:SpeedMode];
  // CV62 and CV63 have lower numbers than the servo CVs, and would be sent first. Therefore the live setting
  // is ended after these writes are completed (see sendNextPomWritePacketFromQueueCompleted)
  endLiveAfterWrites = YES;
  [self sendQueuedWrites];
  [self updateServoPositionsTab];
}

- (void)endLiveSettingAfterCancel {
  if (!endLiveAfterWrites) return;
  endLiveAfterWrites = NO;
  [self queueEndOfLiveSetting];
  [self sendQueuedWrites];
  [self liveSettingEnded];
}

- (IBAction)pushedServoDone:(id)sender {
  [self readLastInputFromTextFields];
  if ((liveServo == 0) || endLiveAfterWrites) return;
  [self queueEndOfLiveSetting];
  [self sendQueuedWrites];
  [self liveSettingEnded];
}

- (void)queueEndOfLiveSetting {
  [_dccDecoderObject setCv:CV_POM_CONFIG_MODE  withValue:NoMode];
  [_dccDecoderObject setCv:CV_POM_CONFIG_SERVO withValue:0];
  [_tcpConnectionsObject queuePomWritePacketForCV:CV_POM_CONFIG_MODE];
  [_tcpConnectionsObject queuePomWritePacketForCV:CV_POM_CONFIG_SERVO];
}

- (void)liveSettingEnded {
  liveServo = 0;
  [_address setEnabled:YES];
  [_buttonSelect setEnabled:YES];
  [self updateTabs];
}

- (void)showServoValue:(int)value slider:(NSSlider *)slider field:(NSTextField *)field stepper:(NSStepper *)stepper {
  [slider setIntValue:value];
  [field setIntValue:value];
  [stepper setIntValue:value];
}

- (void)updateServoPositionsTab {
  BOOL live = ((liveServo != 0) && !endLiveAfterWrites);
  [self updateServoPopups];
  // Values
  [self showServoValue:[self getServoValue16:MinLow] slider:_sliderServoMin field:_fieldServoMin stepper:_stepperServoMin];
  [self showServoValue:[self getServoValue16:MaxLow] slider:_sliderServoMax field:_fieldServoMax stepper:_stepperServoMax];
  [self showServoValue:[_dccDecoderObject getCv:[self servoCv:Speed]]
                slider:_sliderServoSpeed field:_fieldServoSpeed stepper:_stepperServoSpeed];
  // Radio buttons
  [_radioServoMin   setState:(liveMode == MinMode)];
  [_radioServoMax   setState:(liveMode == MaxMode)];
  [_radioServoSpeed setState:(liveMode == SpeedMode)];
  // The values can only be changed during a live setting
  [_sliderServoMin   setEnabled:live];
  [_sliderServoMax   setEnabled:live];
  [_sliderServoSpeed setEnabled:live];
  [_fieldServoMin    setEnabled:live];
  [_fieldServoMax    setEnabled:live];
  [_fieldServoSpeed  setEnabled:live];
  [_stepperServoMin  setEnabled:live];
  [_stepperServoMax  setEnabled:live];
  [_stepperServoSpeed setEnabled:live];
  [_buttonServoStart    setEnabled:(liveServo == 0)];
  [_buttonServoMiddle   setEnabled:(live && (liveMode != SpeedMode))];
  [_buttonServoDefaults setEnabled:live];
  [_buttonServoCancel   setEnabled:live];
  [_buttonServoDone     setEnabled:live];
  // During a live setting CANCEL and DONE are red: one of both must be chosen to end the live setting
  NSColor *endColor = live ? [NSColor redColor] : [NSColor blackColor];
  [self setButtonTitleFor:_buttonServoCancel toString:@"CANCEL" withColor:endColor];
  [self setButtonTitleFor:_buttonServoDone   toString:@"DONE"   withColor:endColor];
  // Status line
  if (liveServo == 0)          [_textServoStatus setStringValue:@"Not active"];
  else if (endLiveAfterWrites) [_textServoStatus setStringValue:@"Cancel: the original values are written"];
  else [_textServoStatus setStringValue:[NSString stringWithFormat:@"Live setting of servo %d", liveServo]];
}


// ********************************************* SERVO CVS tab **********************************************
// Min, Max and Speed (offset 0..3 and 6) are set on the SERVO POSITIONS tab, and are not written here.
// The new values are used by the decoder after a restart.
- (IBAction)selectedServoCv:(id)sender {
  NSString *newString = [sender stringValue];
  if ((sender == _comboCurveA) || (sender == _checkCurveAReverse))
    [self storeCurve:CurveA combo:_comboCurveA check:_checkCurveAReverse];
  if ((sender == _comboCurveB) || (sender == _checkCurveBReverse))
    [self storeCurve:CurveB combo:_comboCurveB check:_checkCurveBReverse];
  if (sender == _comboInvertServoDir)    [self storeServoCv:InvertServoDir    value:[newString isEqualToString:@"Yes"]];
  if (sender == _comboInvertRelais)      [self storeServoCv:InvertRelais      value:[newString isEqualToString:@"Yes"]];
  if (sender == _comboPulseStartUpValue) [self storeServoCv:PulseStartUpValue value:[newString isEqualToString:@"High"]];
  if (sender == _comboPowerWhenIdle)     [self storeServoCv:PowerWhenIdle     value:[newString isEqualToString:@"Yes"]];
  if (sender == _comboIdlePulseDefault) {
    if ([newString isEqualToString:@"Low"])    [self storeServoCv:IdlePulseDefault value:0];
    if ([newString isEqualToString:@"High"])   [self storeServoCv:IdlePulseDefault value:1];
    if ([newString isEqualToString:@"Pulses"]) [self storeServoCv:IdlePulseDefault value:2];
  }
  if (sender == _comboServoType) {
    for (int i = 0; i < 5; i++) if ([newString isEqualToString:servoTypeNames[i]]) [self storeServoCv:ServoType value:i];
  }
  if (sender == _fieldPowerStartUpDelay) [self storeServoCv:PowerStartUpDelay value:[sender intValue]];
  if (sender == _fieldPulseOnBefore)     [self storeServoCv:PulseOnBefore     value:[sender intValue]];
  if (sender == _fieldPulseOffAfter)     [self storeServoCv:PulseOffAfter     value:[sender intValue]];
  if (sender == _fieldPowerOnBefore)     [self storeServoCv:PowerOnBefore     value:[sender intValue]];
  if (sender == _fieldPowerOffAfter)     [self storeServoCv:PowerOffAfter     value:[sender intValue]];
  [self colorGetButtonServoCvsTab:1];
  [self updateServoCvsTab];
}

- (void)storeServoCv:(int)offset value:(int)value {
  if (value < 0)   value = 0;
  if (value > 255) value = 255;
  [_dccDecoderObject setCv:[self servoCv:offset] withValue:value];
}

- (void)storeCurve:(int)offset combo:(NSComboBox *)combo check:(NSButton *)check {
  // Bit 7: direction (1 = reverse). Bit 6: storage (1 = EEPROM). Bits 5..0: index (PROGMEM: 0..11)
  // CurveB: 0 = symmetric (CurveA in opposite direction). Symmetric + Reverse thus gives lin_A in reverse direction
  NSString *name = [combo stringValue];
  int index = -1;
  int newValue;
  for (int i = 0; i < 12; i++) if ([name isEqualToString:curveNames[i]]) index = i;
  if ([name isEqualToString:@"Symmetric"]) index = 0;
  if (index >= 0) newValue = index;
    else newValue = [_dccDecoderObject getCv:[self servoCv:offset]] & 0x7F;   // EEPROM curve: only the direction changes
  if ([check state]) newValue = newValue | 0x80;
  [self storeServoCv:offset value:newValue];
}

- (void)showCurve:(int)offset combo:(NSComboBox *)combo check:(NSButton *)check {
  uint8_t value = [_dccDecoderObject getCv:[self servoCv:offset]];
  int index = value & 0x3F;
  NSString *newString;
  if ((offset == CurveB) && ((value == 0) || (value == 255))) {
    [combo selectItemWithObjectValue:@"Symmetric"];
    [check setState:0];
    return;
  }
  if (value & 0x40) newString = [NSString stringWithFormat:@"EEPROM %d", index];   // Can be shown, not chosen
  else if (index < 12) newString = curveNames[index];
  else newString = [NSString stringWithFormat:@"%d", value];
  [combo setStringValue:newString];
  [check setState:((value & 0x80) != 0)];
}

- (IBAction)pushedServoCvsTabDefaults:(id)sender {
  // Defaults as in servo_CVs.cpp
  [self readLastInputFromTextFields];
  [self storeServoCv:CurveA            value:2];    // move_A
  [self storeServoCv:CurveB            value:0];    // symmetric
  [self storeServoCv:InvertServoDir    value:0];
  [self storeServoCv:InvertRelais      value:0];
  [self storeServoCv:ServoType         value:0];    // Generic
  [self storeServoCv:PulseStartUpValue value:1];    // High
  [self storeServoCv:PowerStartUpDelay value:5];    // 100 ms
  [self storeServoCv:IdlePulseDefault  value:1];    // High
  [self storeServoCv:PulseOnBefore     value:0];
  [self storeServoCv:PulseOffAfter     value:10];   // 200 ms
  // PowerWhenIdle for ServoType Generic: Yes on the servo-3 board (with No, Generic and Uhlenbrock / Weinert
  // servos do not always work reliably on this board), No on the servo-2 and servo-6 boards
  if ([_dccDecoderObject getCv:DecType] == 21) [self storeServoCv:PowerWhenIdle value:1];
    else [self storeServoCv:PowerWhenIdle value:0];
  [self storeServoCv:PowerOnBefore     value:0];
  [self storeServoCv:PowerOffAfter     value:10];   // 200 ms
  [self colorGetButtonServoCvsTab:1];
  [self updateServoCvsTab];
}

- (IBAction)pushedServoCvsTabGet:(id)sender {
  [self readLastInputFromTextFields];
  [self readServoCvs];
  [self colorGetButtonServoCvsTab:0];
}

- (IBAction)pushedServoCvsTabSet:(id)sender {
  [self readLastInputFromTextFields];
  [_tcpConnectionsObject queuePomWritePacketForCV:[self servoCv:CurveA]];
  [_tcpConnectionsObject queuePomWritePacketForCV:[self servoCv:CurveB]];
  for (int offset = InvertServoDir; offset <= PowerOffAfter; offset++)
    [_tcpConnectionsObject queuePomWritePacketForCV:[self servoCv:offset]];
  [self sendQueuedWrites];
  [self colorGetButtonServoCvsTab:0];
}

- (void)updateServoCvsTab {
  uint8_t value;
  BOOL generic;
  BOOL powerSwitched;
  [self updateServoPopups];
  // Curves
  [self showCurve:CurveA combo:_comboCurveA check:_checkCurveAReverse];
  [self showCurve:CurveB combo:_comboCurveB check:_checkCurveBReverse];
  // Servo
  [_comboInvertServoDir selectItemWithObjectValue:[self yesNoStringForCv:[self servoCv:InvertServoDir]]];
  [_comboInvertRelais   selectItemWithObjectValue:[self yesNoStringForCv:[self servoCv:InvertRelais]]];
  [_comboInvertRelais   setEnabled:([_dccDecoderObject getCv:DecType] == 20)];   // Only the servo-2 board has relays
  value = [_dccDecoderObject getCv:[self servoCv:ServoType]];
  if (value < 5) [_comboServoType setStringValue:servoTypeNames[value]];
    else [_comboServoType setStringValue:[NSString stringWithFormat:@"%d", value]];
  // Pulse / Power
  if ([_dccDecoderObject getCv:[self servoCv:PulseStartUpValue]] == 0) [_comboPulseStartUpValue setStringValue:@"Low"];
    else [_comboPulseStartUpValue setStringValue:@"High"];
  value = [_dccDecoderObject getCv:[self servoCv:IdlePulseDefault]];
  if (value == 0)      [_comboIdlePulseDefault setStringValue:@"Low"];
  else if (value == 1) [_comboIdlePulseDefault setStringValue:@"High"];
  else if (value == 2) [_comboIdlePulseDefault setStringValue:@"Pulses"];
  else [_comboIdlePulseDefault setStringValue:[NSString stringWithFormat:@"%d", value]];
  [_comboPowerWhenIdle selectItemWithObjectValue:[self yesNoStringForCv:[self servoCv:PowerWhenIdle]]];
  [_fieldPowerStartUpDelay setIntValue:[_dccDecoderObject getCv:[self servoCv:PowerStartUpDelay]]];
  [_fieldPulseOnBefore     setIntValue:[_dccDecoderObject getCv:[self servoCv:PulseOnBefore]]];
  [_fieldPulseOffAfter     setIntValue:[_dccDecoderObject getCv:[self servoCv:PulseOffAfter]]];
  [_fieldPowerOnBefore     setIntValue:[_dccDecoderObject getCv:[self servoCv:PowerOnBefore]]];
  [_fieldPowerOffAfter     setIntValue:[_dccDecoderObject getCv:[self servoCv:PowerOffAfter]]];
  // Offset 10..14, 16 and 17 are only used with ServoType 0 (Generic); for the other types the firmware
  // uses fixed values. Offset 16 and 17 have no effect if PowerWhenIdle = 1 (power is not switched).
  generic = ([_dccDecoderObject getCv:[self servoCv:ServoType]] == 0);
  powerSwitched = (generic && ([_dccDecoderObject getCv:[self servoCv:PowerWhenIdle]] == 0));
  [_comboPulseStartUpValue setEnabled:generic];
  [_fieldPowerStartUpDelay setEnabled:generic];
  [_comboIdlePulseDefault  setEnabled:generic];
  [_fieldPulseOnBefore     setEnabled:generic];
  [_fieldPulseOffAfter     setEnabled:generic];
  [_fieldPowerOnBefore     setEnabled:powerSwitched];
  [_fieldPowerOffAfter     setEnabled:powerSwitched];
}

- (void)colorGetButtonServoCvsTab:(int)isRed {
  if (isRed) [self setButtonTitleFor:_buttonServoCvsTabSet toString:@"SET" withColor:[NSColor redColor]];
    else     [self setButtonTitleFor:_buttonServoCvsTabSet toString:@"SET" withColor:[NSColor blackColor]];
}



// ************************************************************************************************************
// ********************************** USER INTERFACE METHODS - INITIALISE TAB **********************************
// ************************************************************************************************************
- (IBAction)enteredNewAddress:(id)sender {
  // NSLog(@"User input - New switch address: %i", [sender intValue]);
  int newValue = [sender intValue];
  if (newValue < 1) {newValue = 1;}
  if (newValue > maxSwitchAddress) {newValue = maxSwitchAddress;}   // 2026: was 1024
  newValue = newValue - 1;   // entering is 1..1024 (or 1..2048) <-> storing is 0..1023 (or 0..2047)
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
  [_address setStringValue:@""];   // 2026: was setIntValue:0. An empty field corresponds to address -1
  // Check the values from CV1 & CV9, since wrong values make the decoder unaccessable
  int switchAddr = [_dccDecoderObject getSwitchAddress];
  if ((switchAddr >= 1) && (switchAddr <= maxSwitchAddress)) {   // 2026: was 1024
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
  nonInitialisedFound = NO;   // 2026: the decoder is initialised now
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
// 2026: Tabs are found via their identifier (set in MainMenu.xib), instead of via a fixed index.
// This is needed since more than one tab can be hidden. The Switch and Lift tab are shown at index 3,
// the three Servo tabs at index 3, 4 and 5.
NSTabViewItem *holderForTabSwitch;   // identifier = "2"
NSTabViewItem *holderForTabLift;     // identifier = "3"
NSTabViewItem *holderForTabServo;          // identifier = "4"
NSTabViewItem *holderForTabServoPositions; // identifier = "5"
NSTabViewItem *holderForTabServoCvs;       // identifier = "6"

- (void)hideSwitchTab {[self hideTab:&holderForTabSwitch withIdentifier:@"2"];}
- (void)showSwitchTab {[self showTab:&holderForTabSwitch atIndex:3];}
- (void)hideLiftTab   {[self hideTab:&holderForTabLift   withIdentifier:@"3"];}
- (void)showLiftTab   {[self showTab:&holderForTabLift   atIndex:3];}
- (void)hideServoTab          {[self hideTab:&holderForTabServo          withIdentifier:@"4"];}
- (void)hideServoPositionsTab {[self hideTab:&holderForTabServoPositions withIdentifier:@"5"];}
- (void)hideServoCvsTab       {[self hideTab:&holderForTabServoCvs       withIdentifier:@"6"];}
- (void)showServoTab          {[self showTab:&holderForTabServo          atIndex:3];}
- (void)showServoPositionsTab {[self showTab:&holderForTabServoPositions atIndex:4];}
- (void)showServoCvsTab       {[self showTab:&holderForTabServoCvs       atIndex:5];}

- (BOOL)tabView:(NSTabView *)tabView shouldSelectTabViewItem:(NSTabViewItem *)tabViewItem {
  // 2026: During a live setting of a servo, the SERVO POSITIONS tab (identifier "5") can only be left
  // after DONE or CANCEL
  if (liveServo == 0) return YES;
  return [[tabViewItem identifier] isEqual:@"5"];
}

- (void)hideTab:(NSTabViewItem **)holder withIdentifier:(NSString *)identifier {
  if (*holder == nil) {
    NSInteger index = [_windowTabs indexOfTabViewItemWithIdentifier:identifier];
    if (index == NSNotFound) return;
    *holder = [[_windowTabs tabViewItemAtIndex:index] retain];
    [_windowTabs removeTabViewItem:*holder];
  }
}

- (void)showTab:(NSTabViewItem **)holder atIndex:(NSInteger)index {
  if (*holder != nil) {
    [_windowTabs insertTabViewItem:*holder atIndex:index];
    [*holder release];   // insertTabViewItem retains the tab
    *holder = nil;
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
  [[NSUserDefaults standardUserDefaults] setObject:@"192.168.24.212"       forKey:@"defaultIpAddressForSending"];
  [[NSUserDefaults standardUserDefaults] setObject:@"192.168.24.213"       forKey:@"defaultIpAddressForReceiving"];
  [[NSUserDefaults standardUserDefaults] setBool:1                         forKey:@"defaultIpAddressForReceivingIsActive"];
}

// ************************************************************************************************************
// *************************************** dealloc and closing procedures *************************************
// ************************************************************************************************************
- (void)dealloc {[super dealloc];}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {return YES;}
- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {return YES;}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
  // 2026: A live setting of a servo must be ended first, otherwise the decoder keeps ignoring switch commands.
  // Terminating is cancelled now, and requested again once CV62 and CV63 are written (or after 2 seconds).
  if (liveServo != 0) {
    endLiveAfterWrites = NO;
    [self queueEndOfLiveSetting];
    [self liveSettingEnded];
    terminateAfterWrites = YES;
    [self sendQueuedWrites];
    [self performSelector:@selector(terminateNow) withObject:nil afterDelay:2.0];
    return NSTerminateCancel;
  }
  [_tcpConnectionsObject closeTcp];
  return NSTerminateNow;
}

- (void)terminateNow {
  if (!terminateAfterWrites) return;
  terminateAfterWrites = NO;
  [NSApp terminate:nil];
}




@end
