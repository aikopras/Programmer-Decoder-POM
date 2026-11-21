//
//  ControlTab.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  See ControlTab.cpp
//

#ifndef CONTROLTAB_H
#define CONTROLTAB_H

#include "DecoderTab.h"

namespace Ui { class ControlTab; }

class ControlTab : public DecoderTab
{
  Q_OBJECT

public:
  explicit ControlTab(MainWindow *mainWindow, QWidget *parent = nullptr);
  ~ControlTab() override;
  void updateTab() override;
  void colorSetButton(bool red) override;

private:
  Ui::ControlTab *ui;
  void selectedRsRetry();
  void selectedCmdSystem();
  void selectedDecType();
  void pushedDefaults();
  void pushedGet();
  void pushedSet();
};

#endif // CONTROLTAB_H
