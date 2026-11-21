//
//  MainWindow.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of the main window part of AppDelegate.m: the decoder address, SELECT, LED ON and RESTART,
//  the decoder information, the status lines and the tabs.
//  Differences with the macOS version:
//  - Each tab is a separate class (see src/tabs).
//  - The decoder specific tabs are hidden with setTabVisible, instead of being removed and inserted.
//  - Buttons take the values from the fields when they are pushed (macOS: readLastInputFromTextFields).
//

#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "PomConnection.h"
#include "PreferencesDialog.h"
#include "HelpWindow.h"
#include "CvNames.h"
#include "ControlTab.h"
#include "InitialiseTab.h"
#include "CvTab.h"
#include "SwitchTab.h"
#include "LiftTab.h"
#include "ServoTabs.h"
#include "RsBusTab.h"
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>
#include <QCloseEvent>
#include <QTabBar>
#include <QIntValidator>

// ****************************************************************************************************************************************************
// ****************************************************************** INITIALISATION ******************************************************************
// ****************************************************************************************************************************************************
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  setWindowTitle("Programmer Decoder-POM");
  selectedServo = 1;
  liveServo = 0;
  maxSwitchAddress = 1024;                                // Until the command station reports its version
  quitAfterLiveSetting = false;
  preferencesDialog = nullptr;
  helpWindow = nullptr;
  // Step 1: dccDecoder
  dccDecoder.initialise();
  // Step 2: preferences. Create them if they do not exist
  PreferencesDialog::checkPreferences();
  // Step 3: pomConnection
  pomConnection = new PomConnection(&dccDecoder, this);
  connect(pomConnection, &PomConnection::cvReceived, this, &MainWindow::cvReceived);
  connect(pomConnection, &PomConnection::generalStatus, ui->connectionStatus, &QLabel::setText);
  connect(pomConnection, &PomConnection::sendStatus, ui->sendStatus, &QLabel::setText);
  connect(pomConnection, &PomConnection::receiveStatus, ui->receiveStatus, &QLabel::setText);
  connect(pomConnection, &PomConnection::busy, ui->progressIndicator, &QProgressBar::setVisible);
  connect(pomConnection, &PomConnection::maxSwitchAddress, this, &MainWindow::setMaxSwitchAddress);
  connect(pomConnection, &PomConnection::idle, this, [this]() {
    // The end of a run clears the status line; show the message about a non-initialised decoder again
    if (!nonInitialisedMessage.isEmpty()) ui->connectionStatus->setText(nonInitialisedMessage);
    nonInitialisedMessage.clear();
  });
  ui->progressIndicator->setVisible(false);
  ui->connectionStatus->clear();                          // Remove the texts of the .ui file
  ui->sendStatus->clear();
  ui->receiveStatus->clear();
  // Step 4: UI
  // The address field is empty at startup (as in the macOS version); empty means: non-initialised decoder
  ui->address->setValidator(new QIntValidator(1, 1024, ui->address));
  ui->ledOn->setCheckable(true);
  connect(ui->address, &QLineEdit::editingFinished, this, &MainWindow::takeDecoderAddress);
  connect(ui->buttonSelect, &QPushButton::clicked, this, &MainWindow::selectAddressPushed);
  connect(ui->ledOn, &QPushButton::clicked, this, &MainWindow::ledOnPushed);
  connect(ui->buttonRestart, &QPushButton::clicked, this, &MainWindow::restartPushed);
  createMenus();
  addTabs();
  setFixedSize(width(), height() + menuBar()->sizeHint().height());   // The .ui size excludes the menu bar
  // Step 5: open the TCP connection(s)
  pomConnection->open();
  updateTabs();
  // Step 6: look for a non-initialised decoder (see searchNonInitialisedDecoder)
  searchNonInitialisedDecoder();
}

void MainWindow::searchNonInitialisedDecoder()
{
  // Added in September 2020
  // Try if there are non-initilised decoders. If there are, we use the "version" CV to select
  // the appropriate algorithm to determine the value of CV1 during initialisation
  // Set the address of the uninitilised decoder to -1, which results in a PoM message with address 6999
  // Since reading CV values take time, we first read the "version" CV.
  // Added in November 2026: called at startup, and by SELECT with an empty address field. If a decoder answers,
  // cvReceived reports this and shows the INITIALISE tab.
  dccDecoder.initialise();                                // Do not show the CVs of the previous decoder
  dccDecoder.decoderAddress = -1;
  pomConnection->openIfClosed();
  pomConnection->verifyCv(CV::version);
  // But in addition we read all other CVs to allow the appropriate TABs (switch, servo, ...) to be displayed.
  readAllCvs();
  updateTabs();
}

MainWindow::~MainWindow()
{
  pomConnection->close();
  delete ui;
}

void MainWindow::createMenus()
{
  // On macOS Qt moves Preferences, Quit and About to the application menu
  QMenu *fileMenu = menuBar()->addMenu("&File");
  QAction *preferencesAction = fileMenu->addAction("Preferences...", this, &MainWindow::showPreferences);
  preferencesAction->setMenuRole(QAction::PreferencesRole);
  preferencesAction->setShortcut(QKeySequence::Preferences);
  fileMenu->addSeparator();
  QAction *quitAction = fileMenu->addAction("Quit", this, &QWidget::close);
  quitAction->setMenuRole(QAction::QuitRole);
  quitAction->setShortcut(QKeySequence::Quit);
  QMenu *helpMenu = menuBar()->addMenu("&Help");
  QAction *helpAction = helpMenu->addAction("Programmer Decoder-POM Help", this, &MainWindow::showHelp);
  helpAction->setShortcut(QKeySequence::HelpContents);
  QAction *aboutAction = helpMenu->addAction("About Programmer Decoder-POM", this, &MainWindow::showAbout);
  aboutAction->setMenuRole(QAction::AboutRole);
}

void MainWindow::addTabs()
{
  // Same order as TabIndex
  tabs.append(new ControlTab(this));
  initialiseTab = new InitialiseTab(this);
  tabs.append(initialiseTab);
  tabs.append(new CvTab(this));
  tabs.append(new SwitchTab(this));
  tabs.append(new LiftTab(this));
  tabs.append(new ServoTab(this));
  servoPositionsTab = new ServoPositionsTab(this);
  tabs.append(servoPositionsTab);
  tabs.append(new ServoCvsTab(this));
  tabs.append(new RsBusTab(this));
  const char *names[] = {"Control", "Initialise", "CV", "Switch", "Lift", "Servo", "Servo positions", "Servo CVs", "RS-Bus"};
  for (int i = 0; i < tabs.size(); i++) ui->windowTabs->addTab(tabs[i], names[i]);
  // Bold texts for the tab names and all buttons (the standard Windows font looks rather thin)
  QFont font = ui->windowTabs->tabBar()->font();
  font.setBold(true);
  ui->windowTabs->tabBar()->setFont(font);
  for (QPushButton *button : findChildren<QPushButton *>()) {
    font = button->font();
    font.setBold(true);
    button->setFont(font);
  }
}

void MainWindow::setMaxSwitchAddress(int address)
{
  maxSwitchAddress = address;
  ui->address->setValidator(new QIntValidator(1, address, ui->address));
  ui->label11->setText(QString("1..%1").arg(address));
  initialiseTab->setMaxSwitchAddress(address);
}

void MainWindow::updateTabs()
{
  updateMainWindow();
  for (DecoderTab *tab : tabs) tab->updateTab();
  // Depending on decoder type, show the SWITCH, LIFT or the three SERVO tabs
  uint8_t decType = dccDecoder.getCv(CV::DecType);
  bool isServo = ((decType == 20) || (decType == 21) || (decType == 22));
  ui->windowTabs->setTabVisible(SwitchIndex, decType == 16);
  ui->windowTabs->setTabVisible(LiftIndex, decType == 24);
  ui->windowTabs->setTabVisible(ServoIndex, isServo);
  ui->windowTabs->setTabVisible(ServoPositionsIndex, isServo);
  ui->windowTabs->setTabVisible(ServoCvsIndex, isServo);
  // During a live setting of a servo, the SERVO POSITIONS tab can only be left after DONE or CANCEL,
  // and another decoder can not be selected
  for (int i = 0; i < tabs.size(); i++) ui->windowTabs->setTabEnabled(i, (liveServo == 0) || (i == ServoPositionsIndex));
  ui->address->setEnabled(liveServo == 0);
  ui->buttonSelect->setEnabled(liveServo == 0);
}

void MainWindow::queueTypeSpecificCvs()
{
  // Called once the decoder type (CV27) is received. Only the CVs shown on the decoder specific tab are queued.
  uint8_t decType = dccDecoder.getCv(CV::DecType);
  if (decType == 16) {   // SWITCH tab
    pomConnection->verifyCv(CV::SendFB);
    pomConnection->verifyCv(CV::AlwaysAct);
  }
  if (decType == 24) {   // LIFT tab
    pomConnection->verifyCv(CV::StartHoming);
    pomConnection->verifyCv(CV::IR_Detect);
    pomConnection->verifyCv(CV::LCD_Display);
    pomConnection->verifyCv(CV::Serial_Line);
  }
  if ((decType == 20) || (decType == 21) || (decType == 22)) {   // SERVO tabs
    pomConnection->verifyCv(CV::SwitchCount);
    pomConnection->verifyCv(CV::SwitchType);
    pomConnection->verifyCv(CV::EEPROM_MAX_SERVOS_INDEX);
    readServoCvs();
  }
}

void MainWindow::readServoCvs()
{
  int firstCv = CV::START_INDEX_SERVO_CVS + CV::NUMBER_OF_SERVO_CVS * (selectedServo - 1);
  for (int offset = 0; offset < CV::NUMBER_OF_SERVO_CVS; offset++) pomConnection->verifyCv(firstCv + offset);
}

void MainWindow::selectServo(int servo)
{
  // Read all CVs of the chosen servo
  selectedServo = servo;
  readServoCvs();
  tabs[ServoCvsIndex]->colorSetButton(false);
  updateTabs();
}


// ****************************************************************************************************************************************************
// ****************************************************** FEEDBACK MESSAGE RECEIVED BY THE POM OBJECT *************************************************
// ****************************************************************************************************************************************************
void MainWindow::cvReceived(int cvNumber, uint8_t cvValue)
{
  dccDecoder.setCv(cvNumber, cvValue);
  // A non-initialised decoder answered (PoM address 6999): report this and go to the INITIALISE tab
  if ((cvNumber == CV::version) && (dccDecoder.decoderAddress == -1)) {
    nonInitialisedMessage = QString("Non-initialised decoder found (software version %1)").arg(cvValue);
    ui->connectionStatus->setText(nonInitialisedMessage);
    ui->windowTabs->setCurrentIndex(InitialiseIndex);
  }
  // Once the decoder type is known, also read the CVs specific for that type of decoder
  if (cvNumber == CV::DecType) queueTypeSpecificCvs();
  updateTabs();
}


// ****************************************************************************************************************************************************
// ********************************************************* USER INTERFACE METHODS - MAIN WINDOW *****************************************************
// ****************************************************************************************************************************************************
void MainWindow::takeDecoderAddress()
{
  int newValue = ui->address->text().toInt();           // 0 if the field is empty
  if (newValue < 1) {                                     // Empty field: non-initialised decoder (PoM address 6999)
    dccDecoder.decoderAddress = -1;
    pomConnection->openIfClosed();
    return;
  }
  if (newValue > maxSwitchAddress) {newValue = maxSwitchAddress;}
  newValue = newValue - 1;                                // entering is 1..1024 <-> storing is 0..1023
  newValue = newValue >> 2;                               // decoder address = switch address DIV 4
  if (newValue != dccDecoder.decoderAddress) selectedServo = 1;   // other decoder: start with servo 1
  dccDecoder.decoderAddress = newValue;
  pomConnection->openIfClosed();                          // reconnect if the Lenz interface(s) were not available before
}

void MainWindow::selectAddressPushed()
{
  // SELECT with an empty address field searches for a non-initialised decoder
  if (ui->address->text().toInt() < 1) {searchNonInitialisedDecoder(); return;}
  takeDecoderAddress();                                   // Takes also a value that is typed, but not yet entered
  readAllCvs();
}

void MainWindow::ledOnPushed(bool checked)
{
  dccDecoder.setCv(CV::Search, checked ? 1 : 0);
  pomConnection->writeCv(CV::Search);
  updateMainWindow();
}

void MainWindow::restartPushed()
{
  // A restart also ends a live setting of a servo, since CV62 and CV63 only exist in RAM
  if (liveServo != 0) servoPositionsTab->liveSettingEnded();
  dccDecoder.setCv(CV::Restart, 0x0D);
  pomConnection->writeCv(CV::Restart);
  // Read all CVs again, after a delay of 0.3 seconds
  QTimer::singleShot(300, this, &MainWindow::readAllCvs);
}

void MainWindow::readAllCvs()
{
  pomConnection->verifyCv(1);
  for (int cvNumber = 3; cvNumber <= 10; cvNumber++) pomConnection->verifyCv(cvNumber);
  for (int cvNumber = 19; cvNumber <= 32; cvNumber++) pomConnection->verifyCv(cvNumber);
  // The type specific CVs (33 and higher) are queued once the decoder type (CV27) is received
  // Remove possible red color from SET buttons
  for (DecoderTab *tab : tabs) tab->colorSetButton(false);
}

void MainWindow::setAddressField(int switchAddress)
{
  // 0 (after initialising a decoder) is shown as an empty field
  ui->address->setText((switchAddress > 0) ? QString::number(switchAddress) : QString());
}

void MainWindow::updateMainWindow()
{
  ui->DecoderVendor->setText(dccDecoder.decoderVendor());
  ui->DecoderVersion->setText(dccDecoder.decoderVersion());
  ui->DecoderType->setText(dccDecoder.decoderType());
  ui->DecoderSubType->setText(dccDecoder.decoderSubType());
  ui->DecoderErrors->setText(dccDecoder.decoderErrors());
  bool ledIsOn = (dccDecoder.getCv(CV::Search) != 0);
  ui->ledOn->setChecked(ledIsOn);
  ui->ledOn->setStyleSheet(ledIsOn ? "color: red" : "");
}


void MainWindow::closeEvent(QCloseEvent *event)
{
  // A live setting of a servo must be ended first, otherwise the decoder keeps ignoring switch commands.
  // Quitting is postponed until CV62 and CV63 are written (or at most 2 seconds)
  if ((liveServo != 0) && !quitAfterLiveSetting) {
    quitAfterLiveSetting = true;
    servoPositionsTab->endLiveSetting();
    connect(pomConnection, &PomConnection::idle, this, &QWidget::close);
    QTimer::singleShot(2000, this, &QWidget::close);
    event->ignore();
    return;
  }
  event->accept();
}


// ****************************************************************************************************************************************************
// ******************************************************************** MENUS *************************************************************************
// ****************************************************************************************************************************************************
void MainWindow::showPreferences()
{
  if (preferencesDialog == nullptr) preferencesDialog = new PreferencesDialog(this);
  preferencesDialog->show();
  preferencesDialog->raise();
}

void MainWindow::showHelp()
{
  if (helpWindow == nullptr) helpWindow = new HelpWindow(this);
  helpWindow->show();
  helpWindow->raise();
}

void MainWindow::showAbout()
{
  QMessageBox::about(this, "About Programmer Decoder-POM",
    "<b>Programmer Decoder-POM</b><br>Version " APP_VERSION "<br><br>"
    "Programs the CVs of my DCC decoders via PoM, using a Lenz LAN/USB interface (23151).<br><br>"
    "Qt version: " + QString(qVersion()) + "<br><br>"
    "Copyright (c) 2026 by Aiko Pras");
}
