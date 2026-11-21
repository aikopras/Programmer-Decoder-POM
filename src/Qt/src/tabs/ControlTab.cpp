//
//  ControlTab.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  CONTROL tab: RSRetry (CV20), CmdStation (CV19) and DecType (CV27).
//  Same as the CONTROL tab of the macOS version (AppDelegate.m). The combo boxes react on a choice of the user
//  (signal activated), not on updateTab.
//

#include "ControlTab.h"
#include "ui_controltab.h"
#include "MainWindow.h"
#include "DccDecoder.h"
#include "PomConnection.h"
#include "CvNames.h"

// The texts of the DecType combo box, with their CV27 values
static const struct {const char *name; int value;} decTypes[] = {
  {"switch", 16}, {"switch plus", 17}, {"servo-2", 20}, {"servo-3", 21}, {"servo-6", 22}, {"lift", 24},
  {"relays-4", 32}, {"relays-16", 33}, {"function", 64}, {"safety", 128}
};

ControlTab::ControlTab(MainWindow *mainWindow, QWidget *parent) : DecoderTab(mainWindow, parent), ui(new Ui::ControlTab)
{
  ui->setupUi(this);
  connect(ui->comboRsRetry, &QComboBox::activated, this, &ControlTab::selectedRsRetry);
  connect(ui->comboCmdSystem, &QComboBox::activated, this, &ControlTab::selectedCmdSystem);
  connect(ui->comboDecType, &QComboBox::activated, this, &ControlTab::selectedDecType);
  connect(ui->buttonControlTabDefaults, &QPushButton::clicked, this, &ControlTab::pushedDefaults);
  connect(ui->buttonControlTabGet, &QPushButton::clicked, this, &ControlTab::pushedGet);
  connect(ui->buttonControlTabSet, &QPushButton::clicked, this, &ControlTab::pushedSet);
}

ControlTab::~ControlTab()
{
  delete ui;
}

void ControlTab::selectedRsRetry()
{
  decoder->setCv(CV::RSRetry, ui->comboRsRetry->currentText().toInt());
  colorSetButton(true);
  mainWindow->updateTabs();                               // RSRetry is also shown on the RS-Bus tab
}

void ControlTab::selectedCmdSystem()
{
  QString newString = ui->comboCmdSystem->currentText();
  uint8_t newValue = 1;                                   // Lenz is the default
  if (newString == "standard") newValue = 0;
  decoder->setCv(CV::CmdStation, newValue);
  colorSetButton(true);
}

void ControlTab::selectedDecType()
{
  QString newString = ui->comboDecType->currentText();
  uint8_t newValue = 16;                                  // Select the default value
  for (const auto &decType : decTypes) if (newString == decType.name) newValue = decType.value;
  decoder->setCv(CV::DecType, newValue);
  colorSetButton(true);
  mainWindow->updateTabs();
}

void ControlTab::pushedDefaults()
{
  decoder->setCv(CV::RSRetry, 0);
  decoder->setCv(CV::CmdStation, 1);
  // Note: current versions of the SWITCH / RELAYS4 decoders do not support extension boards, and can therefore not be changed
  colorSetButton(true);
  mainWindow->updateTabs();
}

void ControlTab::pushedGet()
{
  pom->verifyCv(CV::RSRetry);
  pom->verifyCv(CV::CmdStation);
  pom->verifyCv(CV::DecType);
  colorSetButton(false);
}

void ControlTab::pushedSet()
{
  // DecType is not written (see pushedDefaults)
  pom->writeCv(CV::RSRetry);
  pom->writeCv(CV::CmdStation);
  colorSetButton(false);
}

void ControlTab::updateTab()
{
  // RSRetry
  ui->comboRsRetry->setCurrentText(QString::number(decoder->getCv(CV::RSRetry)));
  // Command System
  ui->comboCmdSystem->setCurrentText((decoder->getCv(CV::CmdStation) == 0) ? "standard" : "Lenz");
  // DecType
  QString newString = "switch";                           // default
  for (const auto &decType : decTypes) if (decoder->getCv(CV::DecType) == decType.value) newString = decType.name;
  ui->comboDecType->setCurrentText(newString);
}

void ControlTab::colorSetButton(bool red)
{
  setButtonRed(ui->buttonControlTabSet, red);
}
