//
//  InitialiseTab.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  INITIALISE tab: gives a non-initialised decoder (which listens to loco address 6999) its address.
//  Same as the INITIALISE tab of the macOS version (AppDelegate.m).
//

#include "InitialiseTab.h"
#include "ui_initialisetab.h"
#include "MainWindow.h"
#include "DccDecoder.h"
#include "PomConnection.h"
#include "CvNames.h"
#include <QTimer>

InitialiseTab::InitialiseTab(MainWindow *mainWindow, QWidget *parent) : DecoderTab(mainWindow, parent), ui(new Ui::InitialiseTab)
{
  ui->setupUi(this);
  explanation = ui->label3->text();
  setMaxSwitchAddress(1024);                              // Until the command station reports its version
  connect(ui->addressNew, &QSpinBox::editingFinished, this, &InitialiseTab::enteredNewAddress);
  connect(ui->buttonIniTabSet, &QPushButton::clicked, this, &InitialiseTab::pushedSet);
}

InitialiseTab::~InitialiseTab()
{
  delete ui;
}

void InitialiseTab::enteredNewAddress()
{
  int newValue = ui->addressNew->value();
  newValue = newValue - 1;                                // entering is 1..1024 <-> storing is 0..1023
  newValue = newValue >> 2;                               // decoder address = switch address DIV 4
  // Modified September 2020: Change for newer decoder software the value of CV1
  // CV1 should be in the range 0..63.
  // However, according to RCN213, for the first handheld address (switch = 1) CV1 should become 1.
  // In my original decoder software (before 2020, version <= 10) handheld address = 1 resulted in CV1 = 0
  // To accomodate both versions, at startup this program tries to read CV values using loco address 6999
  // If a non-initialised decoder responds, we know the decoder's software version and can use the
  // appropriate algorithm to set CV1
  if (decoder->getCv(CV::version) > 10) {
    decoder->setCv(CV::myAddrL, (newValue + 1) & 0b00111111);
    decoder->setCv(CV::myAddrH, ((newValue + 1) >> 6) & 0b00000111);
  }
  else {
    decoder->setCv(CV::myAddrL, newValue & 0b00111111);
    decoder->setCv(CV::myAddrH, (newValue >> 6) & 0b00000111);
  }
  // Set the RS-Bus address
  if (newValue <= 128) decoder->setCv(CV::myRSAddr, newValue + 1);
    else decoder->setCv(CV::myRSAddr, 0);
  mainWindow->updateTabs();
  colorSetButton(true);
}

void InitialiseTab::pushedSet()
{
  enteredNewAddress();                                    // Takes also a value that is typed, but not yet entered
  // Set the address of the uninitilised decoder to -1, which will result in a PoM message with address 6999
  decoder->decoderAddress = -1;
  decoder->setCv(CV::Restart, 1);
  mainWindow->setAddressField(0);
  // Check the values from CV1 & CV9, since wrong values make the decoder unaccessable
  int switchAddr = decoder->getSwitchAddress();
  if ((switchAddr >= 1) && (switchAddr <= maxSwitchAddress)) {
    pom->writeCv(CV::myAddrL);
    pom->writeCv(CV::myAddrH);
    pom->writeCv(CV::myRSAddr);
    pom->writeCv(CV::Restart);
    // Update the decoder address, but wait one second to allow all PoM messages to be send using the "old" address for non-initialised decoders
    QTimer::singleShot(1000, this, &InitialiseTab::updateDecoderAddress);
    colorSetButton(false);
  }
  mainWindow->updateTabs();
}

void InitialiseTab::setMaxSwitchAddress(int address)
{
  maxSwitchAddress = address;
  ui->addressNew->setRange(1, address);
  ui->label7->setText(QString("1..%1").arg(address));
  QString text = explanation;
  text.replace("1...1024", QString("1...%1").arg(address));
  text.replace("... 1021", QString("... %1").arg(address - 3));
  ui->label3->setText(text);
}

void InitialiseTab::updateDecoderAddress()
{
  decoder->decoderAddress = decoder->getDecoderAddress();
  mainWindow->setAddressField(decoder->getSwitchAddress());
}

void InitialiseTab::updateTab()
{
  ui->addressNew->setValue(decoder->getSwitchAddress());
  ui->iniTabDecoderAddress->setNum(decoder->getDecoderAddress());
  int rsAddress = decoder->getCv(CV::myRSAddr);
  if (rsAddress <= 128) ui->iniTabRsAddress->setNum(rsAddress);
    else ui->iniTabRsAddress->setText("None");
}

void InitialiseTab::colorSetButton(bool red)
{
  setButtonRed(ui->buttonIniTabSet, red);
}
