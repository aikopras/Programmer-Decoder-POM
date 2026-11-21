//
//  SwitchTab.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  SWITCH tab: the switch-on times T_on_F1..4 (CV3..6), SkipUnEven (CV21), SendFB (CV33) and AlwaysAct (CV34).
//  Same as the SWITCH tab of the macOS version (AppDelegate.m).
//  The times are shown in seconds (CV value / 50, thus steps of 20 ms). The sliders hold the CV value itself (0..255).
//

#include "SwitchTab.h"
#include "ui_switchtab.h"
#include "MainWindow.h"
#include "DccDecoder.h"
#include "PomConnection.h"
#include "CvNames.h"
#include <QSignalBlocker>

SwitchTab::SwitchTab(MainWindow *mainWindow, QWidget *parent) : DecoderTab(mainWindow, parent), ui(new Ui::SwitchTab)
{
  ui->setupUi(this);
  connectDelay(CV::T_on_F1, ui->cv3Slider, ui->cv3Text);
  connectDelay(CV::T_on_F2, ui->cv4Slider, ui->cv4Text);
  connectDelay(CV::T_on_F3, ui->cv5Slider, ui->cv5Text);
  connectDelay(CV::T_on_F4, ui->cv6Slider, ui->cv6Text);
  connect(ui->comboSkipUnEven, &QComboBox::textActivated, this, [this](const QString &text) {storeYesNoCv(CV::SkipUnEven, text);});
  connect(ui->comboSendFB, &QComboBox::textActivated, this, [this](const QString &text) {storeYesNoCv(CV::SendFB, text);});
  connect(ui->comboAlwaysAct, &QComboBox::textActivated, this, [this](const QString &text) {storeYesNoCv(CV::AlwaysAct, text);});
  connect(ui->buttonSwitchTabDefaults, &QPushButton::clicked, this, &SwitchTab::pushedDefaults);
  connect(ui->buttonSwitchTabGet, &QPushButton::clicked, this, &SwitchTab::pushedGet);
  connect(ui->buttonSwitchTabSet, &QPushButton::clicked, this, &SwitchTab::pushedSet);
}

SwitchTab::~SwitchTab()
{
  delete ui;
}

void SwitchTab::connectDelay(int cvNumber, QSlider *slider, QDoubleSpinBox *field)
{
  // The field reacts on Return / leaving the field and on the arrows, not on every typed character
  field->setKeyboardTracking(false);
  connect(slider, &QSlider::valueChanged, this, [this, cvNumber](int value) {storeSwitchDelayCv(cvNumber, value);});
  connect(field, &QDoubleSpinBox::valueChanged, this, [this, cvNumber](double value) {storeSwitchDelayCv(cvNumber, qRound(value * 50));});
}

void SwitchTab::storeSwitchDelayCv(int cvNumber, int cvValue)
{
  if (cvValue > 255) cvValue = 255;
  if (cvValue < 0) cvValue = 0;
  decoder->setCv(cvNumber, cvValue);
  mainWindow->updateTabs();
  colorSetButton(true);
}

void SwitchTab::storeYesNoCv(int cvNumber, const QString &text)
{
  decoder->setCv(cvNumber, (text == "No") ? 0 : 1);       // Yes is the default
  colorSetButton(true);
}

void SwitchTab::pushedDefaults()
{
  decoder->setCv(CV::T_on_F1, 15);                        // Delay 1
  decoder->setCv(CV::T_on_F2, 15);                        // Delay 2
  decoder->setCv(CV::T_on_F3, 15);                        // Delay 3
  decoder->setCv(CV::T_on_F4, 15);                        // Delay 4
  decoder->setCv(CV::SkipUnEven, 1);                        // Default is ON
  decoder->setCv(CV::SendFB, 1);                          // Default is ON
  decoder->setCv(CV::AlwaysAct, 1);                       // Default is ON
  colorSetButton(true);
  updateTab();
}

void SwitchTab::pushedGet()
{
  pom->verifyCv(CV::T_on_F1);
  pom->verifyCv(CV::T_on_F2);
  pom->verifyCv(CV::T_on_F3);
  pom->verifyCv(CV::T_on_F4);
  pom->verifyCv(CV::SkipUnEven);
  pom->verifyCv(CV::SendFB);
  pom->verifyCv(CV::AlwaysAct);
  colorSetButton(false);
}

void SwitchTab::pushedSet()
{
  pom->writeCv(CV::T_on_F1);
  pom->writeCv(CV::T_on_F2);
  pom->writeCv(CV::T_on_F3);
  pom->writeCv(CV::T_on_F4);
  pom->writeCv(CV::SkipUnEven);
  pom->writeCv(CV::SendFB);
  pom->writeCv(CV::AlwaysAct);
  colorSetButton(false);
}

void SwitchTab::updateTab()
{
  // The fields and sliders are set without calling storeSwitchDelayCv
  const struct {int cv; QSlider *slider; QDoubleSpinBox *field;} delays[] = {
    {CV::T_on_F1, ui->cv3Slider, ui->cv3Text}, {CV::T_on_F2, ui->cv4Slider, ui->cv4Text},
    {CV::T_on_F3, ui->cv5Slider, ui->cv5Text}, {CV::T_on_F4, ui->cv6Slider, ui->cv6Text}
  };
  for (const auto &delay : delays) {
    QSignalBlocker blockSlider(delay.slider);
    QSignalBlocker blockField(delay.field);
    delay.slider->setValue(decoder->getCv(delay.cv));
    delay.field->setValue(decoder->getCv(delay.cv) / 50.0);   // in 20 mseconds
  }
  ui->comboSkipUnEven->setCurrentText(yesNoStringForCv(CV::SkipUnEven));
  ui->comboSendFB->setCurrentText(yesNoStringForCv(CV::SendFB));
  ui->comboAlwaysAct->setCurrentText(yesNoStringForCv(CV::AlwaysAct));
}

void SwitchTab::colorSetButton(bool red)
{
  setButtonRed(ui->buttonSwitchTabSet, red);
}
