//
//  InitialiseTab.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  See InitialiseTab.cpp
//

#ifndef INITIALISETAB_H
#define INITIALISETAB_H

#include "DecoderTab.h"

namespace Ui { class InitialiseTab; }

class InitialiseTab : public DecoderTab
{
  Q_OBJECT

public:
  explicit InitialiseTab(MainWindow *mainWindow, QWidget *parent = nullptr);
  ~InitialiseTab() override;
  void updateTab() override;
  void colorSetButton(bool red) override;
  void setMaxSwitchAddress(int address);

private:
  Ui::InitialiseTab *ui;
  int maxSwitchAddress;                   // 1024 or 2048, depending on the command station
  QString explanation;                    // The text of the .ui file, with 1024 as maximum switch address
  void enteredNewAddress();
  void pushedSet();
  void updateDecoderAddress();
};

#endif // INITIALISETAB_H
