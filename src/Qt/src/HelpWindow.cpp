//
//  HelpWindow.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Shows the help pages. These are the same HTML files as those of the macOS version
//  (src/Xcode/Helpfiles/Decoder-POMHelp); CMakeLists.txt puts them as resources (:/help) in the program.
//  Links to other help pages are shown in this window, links to web sites in the web browser.
//

#include "HelpWindow.h"
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

HelpWindow::HelpWindow(QWidget *parent) : QWidget(parent, Qt::Window)
{
  setWindowTitle("Programmer Decoder-POM Help");
  resize(800, 650);
  browser = new QTextBrowser(this);
  browser->setOpenExternalLinks(true);
  browser->setSource(QUrl("qrc:/help/index.html"));
  QPushButton *buttonBack = new QPushButton("Back", this);
  QPushButton *buttonHome = new QPushButton("Index", this);
  buttonBack->setEnabled(false);
  connect(buttonBack, &QPushButton::clicked, browser, &QTextBrowser::backward);
  connect(buttonHome, &QPushButton::clicked, browser, &QTextBrowser::home);
  connect(browser, &QTextBrowser::backwardAvailable, buttonBack, &QPushButton::setEnabled);
  QHBoxLayout *buttons = new QHBoxLayout;
  buttons->addWidget(buttonBack);
  buttons->addWidget(buttonHome);
  buttons->addStretch();
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addLayout(buttons);
  layout->addWidget(browser);
}
