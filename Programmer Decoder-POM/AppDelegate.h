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

@interface AppDelegate : NSObject <NSApplicationDelegate>


// ************************************************************************************************************
// *************************************  METHODS THAT OTHER OBJECTS MAY USE **********************************
// ************************************************************************************************************
// Methods used by the TCP object to signal sending is ready or a (RS-bus) feedback message is received
- (void)sendNextPomVerifyPacketFromQueueCompleted;
- (void)sendNextPomWritePacketFromQueueCompleted;
- (void)feedbackPacketReceivedForAddress:(int)rsBusAddress withCV:(int)cvNumber withValue:(u_int8_t)cvValue;
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

@property (assign) IBOutlet NSComboBox *comboSkipEven;
@property (assign) IBOutlet NSComboBox *comboSendFB;
@property (assign) IBOutlet NSComboBox *comboAlwaysAct;
- (IBAction)selectedSkipEven:(id)sender;
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
@property (assign) IBOutlet NSButton *buttonRsBusTabDefaults;
@property (assign) IBOutlet NSButton *buttonRsBusTabGet;
@property (assign) IBOutlet NSButton *buttonRsBusTabSet;

-(IBAction)selectedRsFec:(id)sender;

// ************************************************************************************************************
// ********************************************** INITIALISE TAB **********************************************
// ************************************************************************************************************
@property (assign) IBOutlet NSTextField *addressNew;
@property (assign) IBOutlet NSTextField *iniTabDecoderAddress;
@property (assign) IBOutlet NSTextField *iniTabRsAddress;

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
