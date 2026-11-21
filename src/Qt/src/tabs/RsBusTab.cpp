//
//  RsBusTab.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  RS-BUS tab: RSRetry (CV20, same as RSFEC), RSParity (CV22), RSPulsCount (CV24) and the read-only error
//  counters ParityErrors (CV31) and PulseErrors (CV32).
//  Same as the RS-BUS tab of the macOS version (AppDelegate.m).
//

#include "RsBusTab.h"
#include "ui_rsbustab.h"
#include "MainWindow.h"
#include "DccDecoder.h"
#include "PomConnection.h"
#include "CvNames.h"

RsBusTab::RsBusTab(MainWindow *mainWindow, QWidget *parent) : DecoderTab(mainWindow, parent), ui(new Ui::RsBusTab)
{
  ui->setupUi(this);
  connect(ui->comboRsFec, &QComboBox::textActivated, this, [this](const QString &text) {storeCv(CV::RSRetry, text);});
  connect(ui->comboRsParity, &QComboBox::textActivated, this, [this](const QString &text) {storeCv(CV::RSParity, text);});
  connect(ui->comboRsPulsCount, &QComboBox::textActivated, this, [this](const QString &text) {storeCv(CV::RSPulsCount, text);});
  connect(ui->buttonRsBusTabDefaults, &QPushButton::clicked, this, &RsBusTab::pushedDefaults);
  connect(ui->buttonRsBusTabGet, &QPushButton::clicked, this, &RsBusTab::pushedGet);
  connect(ui->buttonRsBusTabSet, &QPushButton::clicked, this, &RsBusTab::pushedSet);
}

RsBusTab::~RsBusTab()
{
  delete ui;
}

void RsBusTab::storeCv(int cvNumber, const QString &text)
{
  int newValue = text.toInt();
  if (newValue < 0) newValue = 0;
  if (newValue > 2) newValue = 2;
  decoder->setCv(cvNumber, newValue);
  colorSetButton(true);
  mainWindow->updateTabs();                               // RSRetry is also shown on the Control tab
}

void RsBusTab::pushedDefaults()
{
  // Default values are taken from CvValues.cpp (AP_DCC_Decoder_Core)
  decoder->setCv(CV::RSRetry, 0);                         // No retransmissions
  decoder->setCv(CV::RSParity, 1);                        // Retransmit if data was just transmitted
  decoder->setCv(CV::RSPulsCount, 2);                     // Always retransmit
  colorSetButton(true);
  mainWindow->updateTabs();
}

void RsBusTab::pushedGet()
{
  pom->verifyCv(CV::RSRetry);
  pom->verifyCv(CV::RSParity);
  pom->verifyCv(CV::RSPulsCount);
  pom->verifyCv(CV::ParityErrors);
  pom->verifyCv(CV::PulseErrors);
  colorSetButton(false);
}

void RsBusTab::pushedSet()
{
  // ParityErrors and PulseErrors are read-only, and are therefore not written
  pom->writeCv(CV::RSRetry);
  pom->writeCv(CV::RSParity);
  pom->writeCv(CV::RSPulsCount);
  colorSetButton(false);
}

void RsBusTab::updateTab()
{
  showComboText(ui->comboRsFec, QString::number(decoder->getCv(CV::RSRetry)));
  showComboText(ui->comboRsParity, QString::number(decoder->getCv(CV::RSParity)));
  showComboText(ui->comboRsPulsCount, QString::number(decoder->getCv(CV::RSPulsCount)));
  ui->parityErrorsText->setNum(decoder->getCv(CV::ParityErrors));
  ui->pulseErrorsText->setNum(decoder->getCv(CV::PulseErrors));
}

void RsBusTab::colorSetButton(bool red)
{
  setButtonRed(ui->buttonRsBusTabSet, red);
}
