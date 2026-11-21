//
//  CvTab.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  CV tab: reads (GET) or writes (SET) a single CV, and resets all CVs to their default values (CV8 = 13).
//  Same as the CV tab of the macOS version (AppDelegate.m). The CV number and value are taken from the fields
//  at the moment a button is pushed.
//

#include "CvTab.h"
#include "ui_cvtab.h"
#include "MainWindow.h"
#include "DccDecoder.h"
#include "PomConnection.h"
#include "CvNames.h"
#include <QIntValidator>

CvTab::CvTab(MainWindow *mainWindow, QWidget *parent) : DecoderTab(mainWindow, parent), ui(new Ui::CvTab)
{
  ui->setupUi(this);
  ui->cvNumber->setValidator(new QIntValidator(1, 1024, this));
  ui->cvValue->setValidator(new QIntValidator(0, 255, this));
  connect(ui->buttonCvTabGet, &QPushButton::clicked, this, &CvTab::pushedGet);
  connect(ui->buttonCvTabSet, &QPushButton::clicked, this, &CvTab::pushedSet);
  connect(ui->buttonCvTabResetCvs, &QPushButton::clicked, this, &CvTab::pushedResetCvs);
}

CvTab::~CvTab()
{
  delete ui;
}

void CvTab::pushedGet()
{
  int cvNumber = ui->cvNumber->text().toInt();
  if (cvNumber > 0) pom->verifyCv(cvNumber);
}

void CvTab::pushedSet()
{
  int cvNumber = ui->cvNumber->text().toInt();
  uint8_t cvValue = ui->cvValue->text().toInt();
  if (cvNumber > 0) {
    decoder->setCv(cvNumber, cvValue);
    pom->writeCv(cvNumber);
  }
  mainWindow->updateTabs();
}

void CvTab::pushedResetCvs()
{
  decoder->setCv(CV::VID, 13);
  pom->writeCv(CV::VID);
  mainWindow->updateTabs();
}

void CvTab::updateTab()
{
  int cvNumber = ui->cvNumber->text().toInt();
  if ((cvNumber > 0) && (cvNumber <= DccDecoder::MAX_CVS)) ui->cvValue->setText(QString::number(decoder->getCv(cvNumber)));
}
