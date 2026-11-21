//
//  HelpWindow.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  See HelpWindow.cpp
//

#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <QWidget>

class QTextBrowser;

class HelpWindow : public QWidget
{
  Q_OBJECT

public:
  explicit HelpWindow(QWidget *parent = nullptr);

private:
  QTextBrowser *browser;
};

#endif // HELPWINDOW_H
