//
//  CenteredComboBox.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 21-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  The style draws the combo box without text; the text (or the placeholder text, see DecoderTab::showComboText)
//  is then drawn centered in the text field of the combo box.
//

#include "CenteredComboBox.h"
#include <QStylePainter>
#include <QStyleOptionComboBox>

void CenteredComboBox::paintEvent(QPaintEvent *)
{
  QStylePainter painter(this);
  QStyleOptionComboBox option;
  initStyleOption(&option);
  QString text = currentText();
  QPalette::ColorGroup group = isEnabled() ? QPalette::Active : QPalette::Disabled;
  QColor color = palette().color(group, QPalette::ButtonText);
  if ((currentIndex() < 0) && !placeholderText().isEmpty()) {
    text = placeholderText();
    color = palette().color(group, QPalette::PlaceholderText);
  }
  option.currentText.clear();
  painter.drawComplexControl(QStyle::CC_ComboBox, option);
  QRect field = style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, this);
  painter.setPen(color);
  painter.drawText(field, Qt::AlignCenter, text);
}
