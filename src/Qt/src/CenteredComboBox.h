//
//  CenteredComboBox.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 21-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  A QComboBox that shows the chosen text centered, like the combo boxes of the macOS version.
//  Used in the .ui files for the combo boxes that are centered in MainMenu.xib.
//

#ifndef CENTEREDCOMBOBOX_H
#define CENTEREDCOMBOBOX_H

#include <QComboBox>

class CenteredComboBox : public QComboBox
{
  Q_OBJECT

public:
  explicit CenteredComboBox(QWidget *parent = nullptr) : QComboBox(parent) {}

protected:
  void paintEvent(QPaintEvent *event) override;
};

#endif // CENTEREDCOMBOBOX_H
