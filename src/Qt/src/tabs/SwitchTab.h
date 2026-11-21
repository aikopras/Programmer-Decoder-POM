//
//  SwitchTab.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  See SwitchTab.cpp
//

#ifndef SWITCHTAB_H
#define SWITCHTAB_H

#include "DecoderTab.h"

namespace Ui { class SwitchTab; }
class QSlider;
class QDoubleSpinBox;

class SwitchTab : public DecoderTab
{
  Q_OBJECT

public:
  explicit SwitchTab(MainWindow *mainWindow, QWidget *parent = nullptr);
  ~SwitchTab() override;
  void updateTab() override;
  void colorSetButton(bool red) override;

private:
  Ui::SwitchTab *ui;
  void connectDelay(int cvNumber, QSlider *slider, QDoubleSpinBox *field);
  void storeSwitchDelayCv(int cvNumber, int cvValue);
  void storeYesNoCv(int cvNumber, const QString &text);
  void pushedDefaults();
  void pushedGet();
  void pushedSet();
};

#endif // SWITCHTAB_H
