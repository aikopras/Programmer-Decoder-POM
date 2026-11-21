//
//  DecoderTab.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Base class of all tabs (see DecoderTab.h)
//

#include "DecoderTab.h"
#include "MainWindow.h"
#include "DccDecoder.h"
#include <QPushButton>
#include <QComboBox>

DecoderTab::DecoderTab(MainWindow *mainWindow, QWidget *parent) : QWidget(parent)
{
  this->mainWindow = mainWindow;
  decoder = mainWindow->decoder();
  pom = mainWindow->pom();
}

// macOS: setButtonTitleFor:toString:withColor:
void DecoderTab::setButtonRed(QPushButton *button, bool red)
{
  button->setStyleSheet(red ? "color: red" : "");
}

// Selects the item with this text. A text that is not in the list (for example a value that can not be chosen)
// is shown as placeholder text, without selecting an item (macOS: setStringValue of a NSComboBox)
void DecoderTab::showComboText(QComboBox *combo, const QString &text)
{
  int index = combo->findText(text);
  if (index < 0) combo->setPlaceholderText(text);
  combo->setCurrentIndex(index);
}

QString DecoderTab::yesNoStringForCv(int cvNumber) const
{
  if (decoder->getCv(cvNumber) == 0) return "No";
  return "Yes";
}
