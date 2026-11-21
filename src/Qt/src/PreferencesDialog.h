//
//  PreferencesDialog.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  See PreferencesDialog.cpp
//

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

namespace Ui { class PreferencesDialog; }

class PreferencesDialog : public QDialog
{
  Q_OBJECT

public:
  explicit PreferencesDialog(QWidget *parent = nullptr);
  ~PreferencesDialog() override;
  static void checkPreferences();         // Creates the preferences with default values, if they do not exist

private:
  Ui::PreferencesDialog *ui;
  void saveSettings();
};

#endif // PREFERENCESDIALOG_H
