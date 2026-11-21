//
//  MainWindow.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  See MainWindow.cpp
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include "DccDecoder.h"

namespace Ui { class MainWindow; }
class PomConnection;
class DecoderTab;
class PreferencesDialog;
class ServoPositionsTab;
class HelpWindow;
class InitialiseTab;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

  DccDecoder *decoder() {return &dccDecoder;}
  PomConnection *pom() {return pomConnection;}
  void updateTabs();                      // Shows the current CV values on the main window and all tabs
  void readAllCvs();
  void setAddressField(int switchAddress);
  int selectedServo;                      // 1..6: servo shown on the SERVO POSITIONS and SERVO CVS tabs
  int liveServo;                          // 0: no live setting. 1..6: servo being set live (CV63)
  void selectServo(int servo);            // Used by the popups of the SERVO POSITIONS and SERVO CVS tabs
  void readServoCvs();                    // Reads all CVs of the selected servo

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  Ui::MainWindow *ui;
  DccDecoder dccDecoder;
  PomConnection *pomConnection;
  PreferencesDialog *preferencesDialog;
  HelpWindow *helpWindow;
  // The tabs, in the same order as on the window. The SWITCH, LIFT and SERVO tabs are only visible for that decoder type
  enum TabIndex {ControlIndex, InitialiseIndex, CvIndex, SwitchIndex, LiftIndex, ServoIndex, ServoPositionsIndex,
                 ServoCvsIndex, RsBusIndex};
  QList<DecoderTab *> tabs;
  ServoPositionsTab *servoPositionsTab;
  InitialiseTab *initialiseTab;
  int maxSwitchAddress;                   // 1024 or 2048, depending on the command station
  void setMaxSwitchAddress(int address);
  QString nonInitialisedMessage;          // Shown again at the end of the run that found a non-initialised decoder
  bool quitAfterLiveSetting;              // Quit during a live setting: quit once the live setting is ended

  void createMenus();
  void addTabs();
  void takeDecoderAddress();
  void selectAddressPushed();
  void searchNonInitialisedDecoder();
  void ledOnPushed(bool checked);
  void restartPushed();
  void cvReceived(int cvNumber, uint8_t cvValue);
  void queueTypeSpecificCvs();
  void updateMainWindow();
  void showPreferences();
  void showHelp();
  void showAbout();
};

#endif // MAINWINDOW_H
