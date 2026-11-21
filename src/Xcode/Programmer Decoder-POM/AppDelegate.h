//
//  AppDelegate.h
//  CV Editor for general decoders based on OPENDECODER22
//
//  Created by Aiko Pras on 26-02-12 / 11-03-2013
//  Copyright (c) 2013 by Aiko Pras. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class PreferencesController;
@class DCCDecoderClass;
@class TCPConnectionsClass;

@interface AppDelegate : NSObject <NSApplicationDelegate, NSTabViewDelegate>


// ************************************************************************************************************
// *************************************  METHODS THAT OTHER OBJECTS MAY USE **********************************
// ************************************************************************************************************
// Methods used by the TCP object to signal sending is ready or a (RS-bus) feedback message is received
- (void)sendNextPomVerifyPacketFromQueueCompleted;
- (void)sendNextPomWritePacketFromQueueCompleted;
- (void)feedbackPacketReceivedForAddress:(int)rsBusAddress withCV:(int)cvNumber withValue:(u_int8_t)cvValue;
// 2026: Method used by the TCP object once the software version of the command station is known
- (void)setMaxSwitchAddress:(int)maxAddress;
// Methods used by the AppDelegate and TCP object to signal status
- (void)showGeneralStatus:(NSString *) statustext;
- (void)showSendStatus:(NSString *) statustext;
- (void)showReceiveStatus:(NSString *) statustext;
- (void)progressIndicator:(BOOL) activity;


// ************************************************************************************************************
// ************************************************* GENERAL **************************************************
// ************************************************************************************************************
// Declare properties for the various main objects
@property (retain) DCCDecoderClass *dccDecoderObject;
@property (retain) TCPConnectionsClass *tcpConnectionsObject;
@property (retain) PreferencesController *preferencesController;

// Declare the properties for the main user interface window plus tab part
@property (retain) IBOutlet NSWindow *window;
@property (assign) IBOutlet NSTabView *windowTabs;

// Declare the properties/methods common to all decoders
@property (assign) IBOutlet NSTextField *address;
@property (assign) IBOutlet NSTextField *addressRange;       // 2026: "1..1024" or "1..2048", depending on the command station
@property (assign) IBOutlet NSButton *buttonSelect;         // 2026: disabled during a live setting of a servo
@property (assign) IBOutlet NSTextField *DecoderVendor;
@property (assign) IBOutlet NSTextField *DecoderVersion;
@property (assign) IBOutlet NSTextField *DecoderType;
@property (assign) IBOutlet NSTextField *DecoderSubType;
@property (assign) IBOutlet NSTextField *DecoderErrors;

@property (assign) IBOutlet NSButton *ledOn;

- (IBAction)takeDecoderAddressFrom:(id)sender;
- (IBAction)selectAddressPushed:(id)sender;
- (IBAction)ledOnPushed:(id)sender;
- (IBAction)restartPushed:(id)sender;

// Status bar (bottom part of window) 
@property (assign) IBOutlet NSTextField *connectionStatus;
@property (assign) IBOutlet NSTextField *sendStatus;
@property (assign) IBOutlet NSTextField *receiveStatus;
@property (assign) IBOutlet NSProgressIndicator *progressIndicator;


// ************************************************************************************************************
// ************************************************ CONTROL TAB ***********************************************
// ************************************************************************************************************
@property (assign) IBOutlet NSComboBox *comboRsRetry;
@property (assign) IBOutlet NSComboBox *comboCmdSystem;
@property (assign) IBOutlet NSComboBox *comboDecType;
- (IBAction)selectedRsRetry:(id)sender;
- (IBAction)selectedCmdSystem:(id)sender;
- (IBAction)selectedDecType:(id)sender;

@property (assign) IBOutlet NSButton *buttonControlTabDefaults;
@property (assign) IBOutlet NSButton *buttonControlTabGet;
@property (assign) IBOutlet NSButton *buttonControlTabSet;
- (IBAction)pushedControlTabDefaults:(id)sender;
- (IBAction)pushedControlTabGet:(id)sender;
- (IBAction)pushedControlTabSet:(id)sender;

// ************************************************************************************************************
// ************************************************ SWITCH TAB ************************************************
// ************************************************************************************************************
@property (assign) IBOutlet NSTextField *cv3Text;
@property (assign) IBOutlet NSTextField *cv4Text;
@property (assign) IBOutlet NSTextField *cv5Text;
@property (assign) IBOutlet NSTextField *cv6Text;

@property (assign) IBOutlet NSSlider *cv3Slider;
@property (assign) IBOutlet NSSlider *cv4Slider;
@property (assign) IBOutlet NSSlider *cv5Slider;
@property (assign) IBOutlet NSSlider *cv6Slider;

@property (assign) IBOutlet NSComboBox *comboSwitchTabSkipUnEven;   // CV21 (2026: was comboSkipEven)
@property (assign) IBOutlet NSComboBox *comboSendFB;
@property (assign) IBOutlet NSComboBox *comboAlwaysAct;
- (IBAction)selectedSwitchTabSkipUnEven:(id)sender;
- (IBAction)selectedSendFB:(id)sender;
- (IBAction)selectedAlwaysAct:(id)sender;

@property (assign) IBOutlet NSButton *buttonSwitchTabDefaults;
@property (assign) IBOutlet NSButton *buttonSwitchTabGet;
@property (assign) IBOutlet NSButton *buttonSwitchTabSet;

- (IBAction)selectedCv3:(id)sender;
- (IBAction)selectedCv4:(id)sender;
- (IBAction)selectedCv5:(id)sender;
- (IBAction)selectedCv6:(id)sender;

- (IBAction)pushedSwitchTabDefaults:(id)sender;
- (IBAction)pushedSwitchTabGet:(id)sender;
- (IBAction)pushedSwitchTabSet:(id)sender;


// ************************************************************************************************************
// ************************************************ RS-Bus TAB ************************************************
// ************************************************************************************************************
@property (assign) IBOutlet NSComboBox *comboRsFec;
@property (assign) IBOutlet NSComboBox *comboRsParity;        // 2026
@property (assign) IBOutlet NSComboBox *comboRsPulsCount;     // 2026
@property (assign) IBOutlet NSTextField *parityErrorsText;    // 2026: read-only
@property (assign) IBOutlet NSTextField *pulseErrorsText;     // 2026: read-only
@property (assign) IBOutlet NSButton *buttonRsBusTabDefaults;
@property (assign) IBOutlet NSButton *buttonRsBusTabGet;
@property (assign) IBOutlet NSButton *buttonRsBusTabSet;

-(IBAction)selectedRsFec:(id)sender;
-(IBAction)selectedRsParity:(id)sender;
-(IBAction)selectedRsPulsCount:(id)sender;

- (IBAction)pushedRsBusTabDefaults:(id)sender;
- (IBAction)pushedRsBusTabGet:(id)sender;
- (IBAction)pushedRsBusTabSet:(id)sender;

// ************************************************************************************************************
// ************************************************* LIFT TAB *************************************************
// ************************************************************************************************************
// 2026: CV33..36 of the lift decoder (DecType = 24)
@property (assign) IBOutlet NSComboBox *comboStartHoming;     // CV33
@property (assign) IBOutlet NSComboBox *comboIrDetect;        // CV34
@property (assign) IBOutlet NSComboBox *comboLcdDisplay;      // CV35
@property (assign) IBOutlet NSComboBox *comboSerialLine;      // CV36
@property (assign) IBOutlet NSButton *buttonLiftTabDefaults;
@property (assign) IBOutlet NSButton *buttonLiftTabGet;
@property (assign) IBOutlet NSButton *buttonLiftTabSet;

- (IBAction)selectedStartHoming:(id)sender;
- (IBAction)selectedIrDetect:(id)sender;
- (IBAction)selectedLcdDisplay:(id)sender;
- (IBAction)selectedSerialLine:(id)sender;

- (IBAction)pushedLiftTabDefaults:(id)sender;
- (IBAction)pushedLiftTabGet:(id)sender;
- (IBAction)pushedLiftTabSet:(id)sender;

// ************************************************************************************************************
// ************************************************ SERVO TABS ************************************************
// ************************************************************************************************************
// 2026: Three tabs for the servo decoders (DecType = 20, 21 and 22)
// SERVO tab: general decoder CVs
@property (assign) IBOutlet NSComboBox *comboSkipUnEven;          // CV21
@property (assign) IBOutlet NSComboBox *comboSwitchCount;         // CV35
@property (assign) IBOutlet NSComboBox *comboSwitchType;          // CV36
@property (assign) IBOutlet NSTextField *textServosOnBoard;       // CV64 (read only)
@property (assign) IBOutlet NSButton *buttonServoTabDefaults;
@property (assign) IBOutlet NSButton *buttonServoTabGet;
@property (assign) IBOutlet NSButton *buttonServoTabSet;

- (IBAction)selectedServoTabValue:(id)sender;
- (IBAction)pushedServoTabDefaults:(id)sender;
- (IBAction)pushedServoTabGet:(id)sender;
- (IBAction)pushedServoTabSet:(id)sender;

// SERVO POSITIONS tab: live setting of min, max and speed (CV62 and CV63)
@property (assign) IBOutlet NSPopUpButton *popupServoPositions;
@property (assign) IBOutlet NSButton *radioServoMin;
@property (assign) IBOutlet NSButton *radioServoMax;
@property (assign) IBOutlet NSButton *radioServoSpeed;
@property (assign) IBOutlet NSSlider *sliderServoMin;
@property (assign) IBOutlet NSSlider *sliderServoMax;
@property (assign) IBOutlet NSSlider *sliderServoSpeed;
@property (assign) IBOutlet NSTextField *fieldServoMin;
@property (assign) IBOutlet NSTextField *fieldServoMax;
@property (assign) IBOutlet NSTextField *fieldServoSpeed;
@property (assign) IBOutlet NSStepper *stepperServoMin;
@property (assign) IBOutlet NSStepper *stepperServoMax;
@property (assign) IBOutlet NSStepper *stepperServoSpeed;
@property (assign) IBOutlet NSTextField *textServoStatus;
@property (assign) IBOutlet NSButton *buttonServoStart;
@property (assign) IBOutlet NSButton *buttonServoMiddle;
@property (assign) IBOutlet NSButton *buttonServoDefaults;
@property (assign) IBOutlet NSButton *buttonServoCancel;
@property (assign) IBOutlet NSButton *buttonServoDone;

- (IBAction)selectedServo:(id)sender;            // Used by the popups of the SERVO POSITIONS and SERVO CVS tabs
- (IBAction)selectedServoSetting:(id)sender;     // Radio buttons Min, Max, Speed
- (IBAction)servoSliderMoved:(id)sender;
- (IBAction)servoFieldEntered:(id)sender;
- (IBAction)servoStepperClicked:(id)sender;
- (IBAction)pushedServoStart:(id)sender;
- (IBAction)pushedServoMiddle:(id)sender;
- (IBAction)pushedServoDefaults:(id)sender;
- (IBAction)pushedServoCancel:(id)sender;
- (IBAction)pushedServoDone:(id)sender;

// SERVO CVS tab: the other servo specific CVs (offset 4..17)
@property (assign) IBOutlet NSPopUpButton *popupServoCvs;
@property (assign) IBOutlet NSComboBox *comboCurveA;               // offset 4
@property (assign) IBOutlet NSButton *checkCurveAReverse;          // offset 4, bit 7
@property (assign) IBOutlet NSComboBox *comboCurveB;               // offset 5
@property (assign) IBOutlet NSButton *checkCurveBReverse;          // offset 5, bit 7
@property (assign) IBOutlet NSComboBox *comboInvertServoDir;       // offset 7
@property (assign) IBOutlet NSComboBox *comboInvertRelais;         // offset 8 (servo-2 board only)
@property (assign) IBOutlet NSComboBox *comboServoType;            // offset 9
@property (assign) IBOutlet NSComboBox *comboPulseStartUpValue;    // offset 10
@property (assign) IBOutlet NSTextField *fieldPowerStartUpDelay;   // offset 11
@property (assign) IBOutlet NSComboBox *comboIdlePulseDefault;     // offset 12
@property (assign) IBOutlet NSTextField *fieldPulseOnBefore;       // offset 13
@property (assign) IBOutlet NSTextField *fieldPulseOffAfter;       // offset 14
@property (assign) IBOutlet NSComboBox *comboPowerWhenIdle;        // offset 15
@property (assign) IBOutlet NSTextField *fieldPowerOnBefore;       // offset 16
@property (assign) IBOutlet NSTextField *fieldPowerOffAfter;       // offset 17
@property (assign) IBOutlet NSButton *buttonServoCvsTabDefaults;
@property (assign) IBOutlet NSButton *buttonServoCvsTabGet;
@property (assign) IBOutlet NSButton *buttonServoCvsTabSet;

- (IBAction)selectedServoCv:(id)sender;
- (IBAction)pushedServoCvsTabDefaults:(id)sender;
- (IBAction)pushedServoCvsTabGet:(id)sender;
- (IBAction)pushedServoCvsTabSet:(id)sender;

// ************************************************************************************************************
// ********************************************** INITIALISE TAB **********************************************
// ************************************************************************************************************
@property (assign) IBOutlet NSTextField *addressNew;
@property (assign) IBOutlet NSTextField *iniTabDecoderAddress;
@property (assign) IBOutlet NSTextField *iniTabRsAddress;
@property (assign) IBOutlet NSTextField *iniTabAddressRange; // 2026: "1..1024" or "1..2048", depending on the command station
@property (assign) IBOutlet NSTextField *iniTabExplanation;  // 2026: the text mentions the address range

- (IBAction)enteredNewAddress:(id)sender;


@property (assign) IBOutlet NSButton *buttonIniTabSet;
- (IBAction)pushedIniTabSet:(id)sender;


// ************************************************************************************************************
// ************************************************** CV TAB **************************************************
// ************************************************************************************************************
@property (assign) IBOutlet NSTextField *cvNumber;
@property (assign) IBOutlet NSTextField *cvValue;
- (IBAction)enteredCvNumber:(id)sender;
- (IBAction)enteredCvValue:(id)sender;
- (IBAction)pushedCvTabGet:(id)sender;
- (IBAction)pushedCvTabSet:(id)sender;
- (IBAction)pushedCvTabResetCvs:(id)sender;

// ************************************************************************************************************
// ******************************************** PREFERENCES WINDOW ********************************************
// ************************************************************************************************************
- (IBAction)showPreferences:(id)sender;


@end
