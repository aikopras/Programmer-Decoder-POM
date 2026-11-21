//
//  DecoderTab.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Base class of all tabs. Each tab has its own .ui file (converted from MainMenu.xib of the macOS version)
//  and its own class; the macOS version has all tabs in AppDelegate.m.
//

#ifndef DECODERTAB_H
#define DECODERTAB_H

#include <QWidget>

class MainWindow;
class DccDecoder;
class PomConnection;
class QPushButton;
class QComboBox;

class DecoderTab : public QWidget
{
  Q_OBJECT

public:
  explicit DecoderTab(MainWindow *mainWindow, QWidget *parent = nullptr);
  virtual void updateTab() {}                         // Shows the current CV values (macOS: update...Tab)
  virtual void colorSetButton(bool) {}                // Red SET button: values changed, but not yet written

protected:
  MainWindow *mainWindow;
  DccDecoder *decoder;
  PomConnection *pom;
  static void setButtonRed(QPushButton *button, bool red);
  static void showComboText(QComboBox *combo, const QString &text);   // Also shows a text that is not in the list
  QString yesNoStringForCv(int cvNumber) const;
};

#endif // DECODERTAB_H
