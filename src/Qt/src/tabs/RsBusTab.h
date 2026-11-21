//
//  RsBusTab.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  See RsBusTab.cpp
//

#ifndef RSBUSTAB_H
#define RSBUSTAB_H

#include "DecoderTab.h"

namespace Ui { class RsBusTab; }

class RsBusTab : public DecoderTab
{
  Q_OBJECT

public:
  explicit RsBusTab(MainWindow *mainWindow, QWidget *parent = nullptr);
  ~RsBusTab() override;
  void updateTab() override;
  void colorSetButton(bool red) override;

private:
  Ui::RsBusTab *ui;
  void storeCv(int cvNumber, const QString &text);
  void pushedDefaults();
  void pushedGet();
  void pushedSet();
};

#endif // RSBUSTAB_H
