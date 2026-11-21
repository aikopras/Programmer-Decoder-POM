//
//  LiftTab.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  LIFT tab: CV33..36 of the lift decoder. Values and defaults are taken from CvValues.h / CvValues.cpp (AP_DCC_Decoder_Core)
//  Same as the LIFT tab of the macOS version (AppDelegate.m).
//

#include "LiftTab.h"
#include "ui_lifttab.h"
#include "MainWindow.h"
#include "DccDecoder.h"
#include "PomConnection.h"
#include "CvNames.h"

LiftTab::LiftTab(MainWindow *mainWindow, QWidget *parent) : DecoderTab(mainWindow, parent), ui(new Ui::LiftTab)
{
  ui->setupUi(this);
  connect(ui->comboStartHoming, &QComboBox::textActivated, this, [this](const QString &text) {storeYesNoCv(CV::StartHoming, text);});
  connect(ui->comboIrDetect, &QComboBox::textActivated, this, [this](const QString &text) {storeYesNoCv(CV::IR_Detect, text);});
  connect(ui->comboLcdDisplay, &QComboBox::textActivated, this, [this](const QString &text) {storeYesNoCv(CV::LCD_Display, text);});
  connect(ui->comboSerialLine, &QComboBox::textActivated, this, &LiftTab::selectedSerialLine);
  connect(ui->buttonLiftTabDefaults, &QPushButton::clicked, this, &LiftTab::pushedDefaults);
  connect(ui->buttonLiftTabGet, &QPushButton::clicked, this, &LiftTab::pushedGet);
  connect(ui->buttonLiftTabSet, &QPushButton::clicked, this, &LiftTab::pushedSet);
}

LiftTab::~LiftTab()
{
  delete ui;
}

void LiftTab::storeYesNoCv(int cvNumber, const QString &text)
{
  decoder->setCv(cvNumber, (text == "Yes") ? 1 : 0);
  colorSetButton(true);
}

void LiftTab::selectedSerialLine(const QString &text)
{
  uint8_t newValue = 0;                                   // Select the default value
  if (text == "On") newValue = 1;
  if (text == "Details") newValue = 2;
  decoder->setCv(CV::Serial_Line, newValue);
  colorSetButton(true);
}

void LiftTab::pushedDefaults()
{
  decoder->setCv(CV::StartHoming, 1);                     // Default is ON
  decoder->setCv(CV::IR_Detect, 1);                       // Default is ON
  decoder->setCv(CV::LCD_Display, 0);                     // Default is OFF
  decoder->setCv(CV::Serial_Line, 0);                     // Default is OFF
  colorSetButton(true);
  updateTab();
}

void LiftTab::pushedGet()
{
  pom->verifyCv(CV::StartHoming);
  pom->verifyCv(CV::IR_Detect);
  pom->verifyCv(CV::LCD_Display);
  pom->verifyCv(CV::Serial_Line);
  colorSetButton(false);
}

void LiftTab::pushedSet()
{
  pom->writeCv(CV::StartHoming);
  pom->writeCv(CV::IR_Detect);
  pom->writeCv(CV::LCD_Display);
  pom->writeCv(CV::Serial_Line);
  colorSetButton(false);
}

void LiftTab::updateTab()
{
  ui->comboStartHoming->setCurrentText(yesNoStringForCv(CV::StartHoming));
  ui->comboIrDetect->setCurrentText(yesNoStringForCv(CV::IR_Detect));
  ui->comboLcdDisplay->setCurrentText(yesNoStringForCv(CV::LCD_Display));
  QString newString = "Off";                              // default
  if (decoder->getCv(CV::Serial_Line) == 1) newString = "On";
  if (decoder->getCv(CV::Serial_Line) == 2) newString = "Details";
  ui->comboSerialLine->setCurrentText(newString);
}

void LiftTab::colorSetButton(bool red)
{
  setButtonRed(ui->buttonLiftTabSet, red);
}
