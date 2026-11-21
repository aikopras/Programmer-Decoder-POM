//
//  main.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of Programmer Decoder-POM, for Windows and Linux (and macOS)
//

#include "MainWindow.h"
#include <QApplication>
#include <QIcon>
#include <QStyleFactory>
#include <QPalette>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  app.setApplicationName("Programmer Decoder-POM");
  app.setApplicationVersion(APP_VERSION);
  app.setWindowIcon(QIcon(":/PoM-Decoder-icon.png"));
  // The screens are taken from the macOS version, with fixed positions and sizes. The Fusion style looks the same
  // on Windows and Linux, and the style sheet gives the group boxes a grey background, like the boxes on macOS.
  app.setStyle(QStyleFactory::create("Fusion"));
  // The standard font of macOS is 13 pixels; 10 points is 13.3 pixels (Windows and Linux)
  QFont font = app.font();
  font.setPointSize(10);
  app.setFont(font);
  // Disabled buttons, labels and fields: lighter text, as in the macOS version
  QPalette palette = app.palette();
  for (QPalette::ColorRole role : {QPalette::ButtonText, QPalette::WindowText, QPalette::Text})
    palette.setColor(QPalette::Disabled, role, QColor(175, 175, 175));
  app.setPalette(palette);
  app.setStyleSheet(
    "QGroupBox {background-color: rgba(128, 128, 128, 0.12); border: 1px solid rgba(128, 128, 128, 0.25);"
    "           border-radius: 6px; margin-top: 16px;}"
    "QGroupBox::title {subcontrol-origin: margin; subcontrol-position: top left; left: 8px; padding: 0px 2px;}"
    "QTabWidget::tab-bar {alignment: center;}");
  MainWindow window;
  window.show();
  return app.exec();
}
