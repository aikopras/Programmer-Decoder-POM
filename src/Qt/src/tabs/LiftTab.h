//
//  LiftTab.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  See LiftTab.cpp
//

#ifndef LIFTTAB_H
#define LIFTTAB_H

#include "DecoderTab.h"

namespace Ui { class LiftTab; }

class LiftTab : public DecoderTab
{
  Q_OBJECT

public:
  explicit LiftTab(MainWindow *mainWindow, QWidget *parent = nullptr);
  ~LiftTab() override;
  void updateTab() override;
  void colorSetButton(bool red) override;

private:
  Ui::LiftTab *ui;
  void storeYesNoCv(int cvNumber, const QString &text);
  void selectedSerialLine(const QString &text);
  void pushedDefaults();
  void pushedGet();
  void pushedSet();
};

#endif // LIFTTAB_H
