//
//  PreferencesDialog.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of PreferencesController.m: the IP addresses of the Lenz LAN/USB interface(s).
//  The values are stored with QSettings (Windows: registry, Linux: ~/.config, macOS: plist), and saved as soon as
//  a field is changed. Like the macOS version, the new values are used the next time the program connects.
//

#include "PreferencesDialog.h"
#include "ui_preferencesdialog.h"
#include <QSettings>

PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent), ui(new Ui::PreferencesDialog)
{
  ui->setupUi(this);
  QSettings settings("Programmer Decoder-POM", "Settings");
  ui->ipAddressForSending->setText(settings.value("IpAddressForSending").toString());
  ui->ipAddressForReceiving->setText(settings.value("IpAddressForReceiving").toString());
  ui->receivingIsActive->setChecked(settings.value("IpAddressForReceivingIsActive").toBool());
  connect(ui->ipAddressForSending, &QLineEdit::editingFinished, this, &PreferencesDialog::saveSettings);
  connect(ui->ipAddressForReceiving, &QLineEdit::editingFinished, this, &PreferencesDialog::saveSettings);
  connect(ui->receivingIsActive, &QCheckBox::toggled, this, &PreferencesDialog::saveSettings);
}

PreferencesDialog::~PreferencesDialog()
{
  delete ui;
}

void PreferencesDialog::saveSettings()
{
  QSettings settings("Programmer Decoder-POM", "Settings");
  settings.setValue("IpAddressForSending", ui->ipAddressForSending->text().trimmed());
  settings.setValue("IpAddressForReceiving", ui->ipAddressForReceiving->text().trimmed());
  settings.setValue("IpAddressForReceivingIsActive", ui->receivingIsActive->isChecked());
}

void PreferencesDialog::checkPreferences()
{
  // Check if all preferences exist. If not, create them
  QSettings settings("Programmer Decoder-POM", "Settings");
  if (settings.contains("IpAddressForSending") && settings.contains("IpAddressForReceiving") &&
      settings.contains("IpAddressForReceivingIsActive")) return;
  settings.setValue("IpAddressForSending", "192.168.24.212");
  settings.setValue("IpAddressForReceiving", "192.168.24.213");
  settings.setValue("IpAddressForReceivingIsActive", true);
}
