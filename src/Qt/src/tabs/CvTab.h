//
//  CvTab.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  See CvTab.cpp
//

#ifndef CVTAB_H
#define CVTAB_H

#include "DecoderTab.h"

namespace Ui { class CvTab; }

class CvTab : public DecoderTab
{
  Q_OBJECT

public:
  explicit CvTab(MainWindow *mainWindow, QWidget *parent = nullptr);
  ~CvTab() override;
  void updateTab() override;

private:
  Ui::CvTab *ui;
  void pushedGet();
  void pushedSet();
  void pushedResetCvs();
};

#endif // CVTAB_H
