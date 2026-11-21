//
//  ServoTabs.h
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  See ServoTabs.cpp
//

#ifndef SERVOTABS_H
#define SERVOTABS_H

#include "DecoderTab.h"
#include "CvNames.h"
#include <QStringList>
#include <cstdint>

namespace Ui { class ServoTab; class ServoPositionsTab; class ServoCvsTab; }
class QComboBox;
class QSlider;
class QSpinBox;
class QCheckBox;

// ********************************************* Support methods for the three servo tabs *********************************************
class ServoBase : public DecoderTab
{
  Q_OBJECT

public:
  explicit ServoBase(MainWindow *mainWindow, QWidget *parent = nullptr);

protected:
  int servoCv(int offset) const;                          // CV number of a servo CV of the selected servo
  uint16_t getServoValue16(int offset) const;
  void setServoValue16(int offset, uint16_t value);
  int servosOnBoard() const;
  int switchType() const;
  int servosInUse() const;
  void fillServoPopup(QComboBox *popup);
  static void fillComboBox(QComboBox *combo, const QStringList &items);
};

// ***************************************************************** SERVO tab ******************************************************************
class ServoTab : public ServoBase
{
  Q_OBJECT

public:
  explicit ServoTab(MainWindow *mainWindow, QWidget *parent = nullptr);
  ~ServoTab() override;
  void updateTab() override;
  void colorSetButton(bool red) override;

private:
  Ui::ServoTab *ui;
  void selectedSkipUnEven(const QString &text);
  void selectedSwitchCount(const QString &text);
  void selectedSwitchType(const QString &text);
  void pushedDefaults();
  void pushedGet();
  void pushedSet();
};

// ************************************************************ SERVO POSITIONS tab *************************************************************
class ServoPositionsTab : public ServoBase
{
  Q_OBJECT

public:
  explicit ServoPositionsTab(MainWindow *mainWindow, QWidget *parent = nullptr);
  ~ServoPositionsTab() override;
  void updateTab() override;
  void endLiveSetting();                                  // Writes CV62 = 0 and CV63 = 0 (DONE and quit)
  void liveSettingEnded();                                // Without writing CV62 and CV63 (RESTART)

private:
  Ui::ServoPositionsTab *ui;
  int liveMode;                                           // MinMode, MaxMode or SpeedMode (CV62), as selected by the radio buttons
  uint16_t originalMin;                                   // Values read before START, used by CANCEL
  uint16_t originalMax;
  uint8_t originalSpeed;
  bool pending[SpeedMode + 1];                            // A new value of this mode is written once the queued requests are sent
  void connectValue(int mode, QSlider *slider, QSpinBox *field);
  void selectedServoSetting(int mode);
  void liveValue(int mode, int value);
  void storeLiveValue(int mode, int value);
  void writeLiveValue(int mode);
  void writePendingValues();
  void pushedStart();
  void pushedMiddle();
  void pushedDefaults();
  void pushedCancel();
  void pushedDone();
  static void showServoValue(int value, QSlider *slider, QSpinBox *field);
};

// **************************************************************** SERVO CVS tab ***************************************************************
class ServoCvsTab : public ServoBase
{
  Q_OBJECT

public:
  explicit ServoCvsTab(MainWindow *mainWindow, QWidget *parent = nullptr);
  ~ServoCvsTab() override;
  void updateTab() override;
  void colorSetButton(bool red) override;

private:
  Ui::ServoCvsTab *ui;
  void connectCombo(QComboBox *combo, int offset);
  void connectField(QSpinBox *field, int offset);
  void storeServoCv(int offset, int value);
  void storeCurve(int offset, QComboBox *combo, QCheckBox *check);
  void showCurve(int offset, QComboBox *combo, QCheckBox *check);
  void pushedDefaults();
  void pushedGet();
  void pushedSet();
};

#endif // SERVOTABS_H
