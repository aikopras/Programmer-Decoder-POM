//
//  ServoTabs.cpp
//  Programmer Decoder-POM (Qt)
//
//  Created by Aiko Pras on 20-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Three tabs for the servo decoders (DecType = 20, 21 and 22). The firmware (AVR-Servo-3-6) is leading.
//  - SERVO:           the general decoder CVs (CV21, CV35, CV36 and CV64)
//  - SERVO POSITIONS: live setting of Min, Max and Speed of one servo, via CV62 and CV63
//  - SERVO CVS:       the other CVs of one servo
//  CV number of a servo CV = 65 + 18 x (servo - 1) + offset
//  Same as the SERVO tabs of the macOS version (AppDelegate.m), except:
//  - CANCEL writes the original values and then CV62 = 0 and CV63 = 0 directly after each other. The PoM queue
//    sends the requests in the order in which they were queued (macOS: in ascending CV order, which required
//    ending the live setting after the other writes were completed).
//  - During a live setting a new Min, Max or Speed value is written immediately if no other requests are queued.
//    Otherwise it is written once these requests are sent; quick changes (mouse wheel, arrows held down) therefore
//    result in a single write of the last value (macOS: a CV is queued only once).
//

#include "ServoTabs.h"
#include "ui_servotab.h"
#include "ui_servopositionstab.h"
#include "ui_servocvstab.h"
#include "MainWindow.h"
#include "DccDecoder.h"
#include "PomConnection.h"
#include <QSignalBlocker>

static const QStringList curveNames = {"lin_A", "lin_B", "move_A", "move_B", "sine_A", "sine_B",
                                       "whip_A", "whip_B", "sig_hp0", "sig_hp1", "hp1p", "sine_AB"};

// ****************************************************************************************************************************************************
// ****************************************************************** SUPPORT METHODS *****************************************************************
// ****************************************************************************************************************************************************
ServoBase::ServoBase(MainWindow *mainWindow, QWidget *parent) : DecoderTab(mainWindow, parent)
{
}

int ServoBase::servoCv(int offset) const
{
  return CV::START_INDEX_SERVO_CVS + CV::NUMBER_OF_SERVO_CVS * (mainWindow->selectedServo - 1) + offset;
}

uint16_t ServoBase::getServoValue16(int offset) const
{
  // Min and Max are 16 bit values: low byte at offset, high byte at offset + 1
  return decoder->getCv(servoCv(offset)) + 256 * decoder->getCv(servoCv(offset + 1));
}

void ServoBase::setServoValue16(int offset, uint16_t value)
{
  decoder->setCv(servoCv(offset), value & 0xFF);
  decoder->setCv(servoCv(offset + 1), value >> 8);
}

int ServoBase::servosOnBoard() const
{
  // CV64 holds the maximum number of servos of the board. If not (yet) available, use the decoder type
  int number = decoder->getCv(CV::EEPROM_MAX_SERVOS_INDEX);
  if ((number >= 1) && (number <= 6)) return number;
  if (decoder->getCv(CV::DecType) == 20) return 2;
  if (decoder->getCv(CV::DecType) == 21) return 3;
  return 6;
}

int ServoBase::switchType() const
{
  // The SwitchType as used by the firmware. The servo-2 board has no SwitchType.
  // Hosentrager requires 4 servos; on a board with less servos the decoder operates as a normal decoder
  int type = decoder->getCv(CV::SwitchType);
  if (decoder->getCv(CV::DecType) == 20) return NORMAL;
  if ((type == HOSENTRAGER) && (servosOnBoard() < 4)) return NORMAL;
  if (type > HOSENTRAGER) return NORMAL;
  return type;
}

int ServoBase::servosInUse() const
{
  // DKW uses 3 servos and Hosentrager 4. Otherwise SwitchCount: 0, 255 or above the maximum = board maximum
  int maxServos = servosOnBoard();
  int count = decoder->getCv(CV::SwitchCount);
  if (switchType() == DKW) return 3;
  if (switchType() == HOSENTRAGER) return 4;
  if ((count == 0) || (count == 255) || (count > maxServos)) count = maxServos;
  return count;
}

void ServoBase::fillServoPopup(QComboBox *popup)
{
  // The popups of the SERVO POSITIONS and SERVO CVS tabs show the same servo.
  // During a live setting another servo can not be chosen; first DONE or CANCEL.
  if ((mainWindow->selectedServo > servosInUse()) && (mainWindow->liveServo == 0)) mainWindow->selectedServo = 1;
  QStringList items;
  for (int i = 1; i <= servosInUse(); i++) items.append(QString("Servo %1").arg(i));
  fillComboBox(popup, items);
  popup->setCurrentIndex(mainWindow->selectedServo - 1);
  popup->setEnabled(mainWindow->liveServo == 0);
}

void ServoBase::fillComboBox(QComboBox *combo, const QStringList &items)
{
  // Only refill if the items differ, so an open list is not disturbed by every feedback message
  QStringList current;
  for (int i = 0; i < combo->count(); i++) current.append(combo->itemText(i));
  if (current == items) return;
  combo->clear();
  combo->addItems(items);
}


// ****************************************************************************************************************************************************
// ********************************************************************* SERVO TAB ********************************************************************
// ****************************************************************************************************************************************************
ServoTab::ServoTab(MainWindow *mainWindow, QWidget *parent) : ServoBase(mainWindow, parent), ui(new Ui::ServoTab)
{
  ui->setupUi(this);
  connect(ui->comboSkipUnEven, &QComboBox::textActivated, this, &ServoTab::selectedSkipUnEven);
  connect(ui->comboSwitchCount, &QComboBox::textActivated, this, &ServoTab::selectedSwitchCount);
  connect(ui->comboSwitchType, &QComboBox::textActivated, this, &ServoTab::selectedSwitchType);
  connect(ui->buttonServoTabDefaults, &QPushButton::clicked, this, &ServoTab::pushedDefaults);
  connect(ui->buttonServoTabGet, &QPushButton::clicked, this, &ServoTab::pushedGet);
  connect(ui->buttonServoTabSet, &QPushButton::clicked, this, &ServoTab::pushedSet);
}

ServoTab::~ServoTab()
{
  delete ui;
}

void ServoTab::selectedSkipUnEven(const QString &text)
{
  decoder->setCv(CV::SkipUnEven, (text == "Yes") ? 1 : 0);
  colorSetButton(true);
  mainWindow->updateTabs();
}

void ServoTab::selectedSwitchCount(const QString &text)
{
  int newValue = 0;                                       // 0 = Maximum
  if (text != "Maximum") newValue = text.toInt();
  decoder->setCv(CV::SwitchCount, newValue);
  colorSetButton(true);
  mainWindow->updateTabs();                               // SwitchCount and SwitchType determine the number of servos in the popups
}

void ServoTab::selectedSwitchType(const QString &text)
{
  int newValue = NORMAL;
  if (text == "DKW") newValue = DKW;
  if (text == "Hosentrager") newValue = HOSENTRAGER;
  decoder->setCv(CV::SwitchType, newValue);
  colorSetButton(true);
  mainWindow->updateTabs();
}

void ServoTab::pushedDefaults()
{
  decoder->setCv(CV::SkipUnEven, 1);                      // Default for servo decoders (CvValues.cpp)
  decoder->setCv(CV::SwitchCount, 0);                     // Board maximum
  decoder->setCv(CV::SwitchType, NORMAL);
  colorSetButton(true);
  mainWindow->updateTabs();
}

void ServoTab::pushedGet()
{
  pom->verifyCv(CV::SkipUnEven);
  pom->verifyCv(CV::SwitchCount);
  pom->verifyCv(CV::SwitchType);
  pom->verifyCv(CV::EEPROM_MAX_SERVOS_INDEX);
  colorSetButton(false);
}

void ServoTab::pushedSet()
{
  // CV64 is read only. The servo-2 board has no SwitchType
  pom->writeCv(CV::SkipUnEven);
  pom->writeCv(CV::SwitchCount);
  if (decoder->getCv(CV::DecType) != 20) pom->writeCv(CV::SwitchType);
  colorSetButton(false);
}

void ServoTab::updateTab()
{
  int maxServos = servosOnBoard();
  int count = decoder->getCv(CV::SwitchCount);
  QStringList items;
  QString newString;
  // SkipUnEven
  ui->comboSkipUnEven->setCurrentText(yesNoStringForCv(CV::SkipUnEven));
  // SwitchType: DKW requires 3 servos, Hosentrager 4 (servo-6 board only). The servo-2 board has no SwitchType
  items.append("Normal");
  if (maxServos >= 3) items.append("DKW");
  if (maxServos >= 4) items.append("Hosentrager");
  fillComboBox(ui->comboSwitchType, items);
  newString = "Normal";
  if (switchType() == DKW) newString = "DKW";
  if (switchType() == HOSENTRAGER) newString = "Hosentrager";
  ui->comboSwitchType->setCurrentText(newString);
  ui->comboSwitchType->setEnabled(decoder->getCv(CV::DecType) != 20);
  // SwitchCount: only used if SwitchType is Normal. For DKW and Hosentrager the firmware determines the number
  items = QStringList("Maximum");
  for (int i = 1; i <= maxServos; i++) items.append(QString::number(i));
  fillComboBox(ui->comboSwitchCount, items);
  newString = "Maximum";
  if ((count >= 1) && (count <= maxServos)) newString = QString::number(count);
  ui->comboSwitchCount->setCurrentText(newString);
  ui->comboSwitchCount->setEnabled(switchType() == NORMAL);
  // CV64
  ui->textServosOnBoard->setNum(maxServos);
}

void ServoTab::colorSetButton(bool red)
{
  setButtonRed(ui->buttonServoTabSet, red);
}


// ****************************************************************************************************************************************************
// *************************************************************** SERVO POSITIONS TAB ****************************************************************
// ****************************************************************************************************************************************************
// START writes CV62 (mode) and CV63 (servo). Each change of Min, Max or Speed is written immediately (see the header);
// the firmware uses a new Min or Max after its high byte, which is therefore written after the low byte.
// DONE writes CV62 = 0 and CV63 = 0. CANCEL first writes the values read before START, and then CV62 = 0 and CV63 = 0.
ServoPositionsTab::ServoPositionsTab(MainWindow *mainWindow, QWidget *parent) : ServoBase(mainWindow, parent), ui(new Ui::ServoPositionsTab)
{
  ui->setupUi(this);
  liveMode = MinMode;
  originalMin = 0;
  originalMax = 0;
  originalSpeed = 0;
  for (bool &flag : pending) flag = false;
  connect(ui->popupServoPositions, &QComboBox::activated, mainWindow, [mainWindow](int index) {mainWindow->selectServo(index + 1);});
  connect(ui->radioServoMin, &QRadioButton::clicked, this, [this]() {selectedServoSetting(MinMode);});
  connect(ui->radioServoMax, &QRadioButton::clicked, this, [this]() {selectedServoSetting(MaxMode);});
  connect(ui->radioServoSpeed, &QRadioButton::clicked, this, [this]() {selectedServoSetting(SpeedMode);});
  connectValue(MinMode, ui->sliderServoMin, ui->fieldServoMin);
  connectValue(MaxMode, ui->sliderServoMax, ui->fieldServoMax);
  connectValue(SpeedMode, ui->sliderServoSpeed, ui->fieldServoSpeed);
  connect(ui->buttonServoStart, &QPushButton::clicked, this, &ServoPositionsTab::pushedStart);
  connect(ui->buttonServoMiddle, &QPushButton::clicked, this, &ServoPositionsTab::pushedMiddle);
  connect(ui->buttonServoDefaults, &QPushButton::clicked, this, &ServoPositionsTab::pushedDefaults);
  connect(ui->buttonServoCancel, &QPushButton::clicked, this, &ServoPositionsTab::pushedCancel);
  connect(ui->buttonServoDone, &QPushButton::clicked, this, &ServoPositionsTab::pushedDone);
  connect(pom, &PomConnection::idle, this, &ServoPositionsTab::writePendingValues);
}

ServoPositionsTab::~ServoPositionsTab()
{
  delete ui;
}

void ServoPositionsTab::connectValue(int mode, QSlider *slider, QSpinBox *field)
{
  // As in the macOS version, a slider reacts when it is released; a field on Return / leaving the field and on the arrows
  slider->setTracking(false);
  field->setKeyboardTracking(false);
  connect(slider, &QSlider::valueChanged, this, [this, mode](int value) {liveValue(mode, value);});
  connect(field, &QSpinBox::valueChanged, this, [this, mode](int value) {liveValue(mode, value);});
}

void ServoPositionsTab::selectedServoSetting(int mode)
{
  liveMode = mode;
  if (mainWindow->liveServo != 0) {
    decoder->setCv(CV::CV_POM_CONFIG_MODE, liveMode);
    pom->writeCv(CV::CV_POM_CONFIG_MODE);
  }
  updateTab();
}

void ServoPositionsTab::liveValue(int mode, int value)
{
  // A change of Min, Max or Speed also selects that mode, so the servo shows the new value
  if (mainWindow->liveServo == 0) return;
  if (mode != liveMode) {
    liveMode = mode;
    decoder->setCv(CV::CV_POM_CONFIG_MODE, liveMode);
    pom->writeCv(CV::CV_POM_CONFIG_MODE);
  }
  storeLiveValue(mode, value);
  if (pom->isIdle()) writeLiveValue(mode);
    else pending[mode] = true;
  updateTab();
}

void ServoPositionsTab::storeLiveValue(int mode, int value)
{
  // Min and Max: 544..2400 us (limits of the Servo-TCA library). Speed: 1..50
  if (mode == SpeedMode) {
    if (value < 1) value = 1;
    if (value > 50) value = 50;                           // The decoder accepts 1..255; in practice 50 is more than enough
    decoder->setCv(servoCv(ServoCv::Speed), value);
    return;
  }
  if (value < 544) value = 544;
  if (value > 2400) value = 2400;
  setServoValue16((mode == MaxMode) ? ServoCv::MaxLow : ServoCv::MinLow, value);
}

void ServoPositionsTab::writeLiveValue(int mode)
{
  pending[mode] = false;
  if (mode == SpeedMode) {pom->writeCv(servoCv(ServoCv::Speed)); return;}
  int offset = (mode == MaxMode) ? ServoCv::MaxLow : ServoCv::MinLow;
  pom->writeCv(servoCv(offset));                          // low byte
  pom->writeCv(servoCv(offset + 1));                      // high byte
}

void ServoPositionsTab::writePendingValues()
{
  // Called once all queued requests are sent
  for (int mode = MinMode; mode <= SpeedMode; mode++) {
    if (pending[mode] && (mainWindow->liveServo != 0)) writeLiveValue(mode);
    pending[mode] = false;
  }
}

void ServoPositionsTab::pushedStart()
{
  if (mainWindow->liveServo != 0) return;
  mainWindow->liveServo = mainWindow->selectedServo;
  // The values of this servo were read after choosing the servo (or after SELECT). Keep them for CANCEL
  originalMin = getServoValue16(ServoCv::MinLow);
  originalMax = getServoValue16(ServoCv::MaxLow);
  originalSpeed = decoder->getCv(servoCv(ServoCv::Speed));
  decoder->setCv(CV::CV_POM_CONFIG_MODE, liveMode);
  decoder->setCv(CV::CV_POM_CONFIG_SERVO, mainWindow->liveServo);
  pom->writeCv(CV::CV_POM_CONFIG_MODE);
  pom->writeCv(CV::CV_POM_CONFIG_SERVO);
  // Another decoder or another tab can only be selected after DONE or CANCEL (see MainWindow::updateTabs)
  mainWindow->updateTabs();
}

void ServoPositionsTab::pushedMiddle()
{
  if ((mainWindow->liveServo == 0) || (liveMode == SpeedMode)) return;
  liveValue(liveMode, 1500);
}

void ServoPositionsTab::pushedDefaults()
{
  // Defaults as in servo_CVs.cpp. The mode (CV62) does not change
  if (mainWindow->liveServo == 0) return;
  storeLiveValue(MinMode, 1300);
  storeLiveValue(MaxMode, 1700);
  storeLiveValue(SpeedMode, 6);
  writeLiveValue(MinMode);
  writeLiveValue(MaxMode);
  writeLiveValue(SpeedMode);
  updateTab();
}

void ServoPositionsTab::pushedCancel()
{
  // The popups are disabled during a live setting, thus selectedServo == liveServo
  if (mainWindow->liveServo == 0) return;
  storeLiveValue(MinMode, originalMin);
  storeLiveValue(MaxMode, originalMax);
  storeLiveValue(SpeedMode, originalSpeed);
  writeLiveValue(MinMode);
  writeLiveValue(MaxMode);
  writeLiveValue(SpeedMode);
  endLiveSetting();
}

void ServoPositionsTab::pushedDone()
{
  if (mainWindow->liveServo == 0) return;
  endLiveSetting();
}

void ServoPositionsTab::endLiveSetting()
{
  // Values that are still pending are written first
  writePendingValues();
  decoder->setCv(CV::CV_POM_CONFIG_MODE, NoMode);
  decoder->setCv(CV::CV_POM_CONFIG_SERVO, 0);
  pom->writeCv(CV::CV_POM_CONFIG_MODE);
  pom->writeCv(CV::CV_POM_CONFIG_SERVO);
  liveSettingEnded();
}

void ServoPositionsTab::liveSettingEnded()
{
  mainWindow->liveServo = 0;
  for (bool &flag : pending) flag = false;
  mainWindow->updateTabs();
}

void ServoPositionsTab::showServoValue(int value, QSlider *slider, QSpinBox *field)
{
  QSignalBlocker blockSlider(slider);
  QSignalBlocker blockField(field);
  slider->setValue(value);
  field->setValue(value);
}

void ServoPositionsTab::updateTab()
{
  bool live = (mainWindow->liveServo != 0);
  fillServoPopup(ui->popupServoPositions);
  // Values
  showServoValue(getServoValue16(ServoCv::MinLow), ui->sliderServoMin, ui->fieldServoMin);
  showServoValue(getServoValue16(ServoCv::MaxLow), ui->sliderServoMax, ui->fieldServoMax);
  showServoValue(decoder->getCv(servoCv(ServoCv::Speed)), ui->sliderServoSpeed, ui->fieldServoSpeed);
  // Radio buttons
  ui->radioServoMin->setChecked(liveMode == MinMode);
  ui->radioServoMax->setChecked(liveMode == MaxMode);
  ui->radioServoSpeed->setChecked(liveMode == SpeedMode);
  // The values can only be changed during a live setting
  ui->sliderServoMin->setEnabled(live);
  ui->sliderServoMax->setEnabled(live);
  ui->sliderServoSpeed->setEnabled(live);
  ui->fieldServoMin->setEnabled(live);
  ui->fieldServoMax->setEnabled(live);
  ui->fieldServoSpeed->setEnabled(live);
  ui->buttonServoStart->setEnabled(!live);
  ui->buttonServoMiddle->setEnabled(live && (liveMode != SpeedMode));
  ui->buttonServoDefaults->setEnabled(live);
  ui->buttonServoCancel->setEnabled(live);
  ui->buttonServoDone->setEnabled(live);
  // During a live setting CANCEL and DONE are red: one of both must be chosen to end the live setting
  setButtonRed(ui->buttonServoCancel, live);
  setButtonRed(ui->buttonServoDone, live);
  // Status line
  if (!live) ui->textServoStatus->setText("Not active");
    else ui->textServoStatus->setText(QString("Live setting of servo %1").arg(mainWindow->liveServo));
}


// ****************************************************************************************************************************************************
// ****************************************************************** SERVO CVS TAB *******************************************************************
// ****************************************************************************************************************************************************
// Min, Max and Speed (offset 0..3 and 6) are set on the SERVO POSITIONS tab, and are not written here.
// The new values are used by the decoder after a restart.
ServoCvsTab::ServoCvsTab(MainWindow *mainWindow, QWidget *parent) : ServoBase(mainWindow, parent), ui(new Ui::ServoCvsTab)
{
  ui->setupUi(this);
  connect(ui->popupServoCvs, &QComboBox::activated, mainWindow, [mainWindow](int index) {mainWindow->selectServo(index + 1);});
  connect(ui->comboCurveA, &QComboBox::activated, this, [this]() {storeCurve(ServoCv::CurveA, ui->comboCurveA, ui->checkCurveAReverse);});
  connect(ui->checkCurveAReverse, &QCheckBox::clicked, this, [this]() {storeCurve(ServoCv::CurveA, ui->comboCurveA, ui->checkCurveAReverse);});
  connect(ui->comboCurveB, &QComboBox::activated, this, [this]() {storeCurve(ServoCv::CurveB, ui->comboCurveB, ui->checkCurveBReverse);});
  connect(ui->checkCurveBReverse, &QCheckBox::clicked, this, [this]() {storeCurve(ServoCv::CurveB, ui->comboCurveB, ui->checkCurveBReverse);});
  connectCombo(ui->comboInvertServoDir, ServoCv::InvertServoDir);
  connectCombo(ui->comboInvertRelais, ServoCv::InvertRelais);
  connectCombo(ui->comboServoType, ServoCv::ServoType);
  connectCombo(ui->comboPulseStartUpValue, ServoCv::PulseStartUpValue);
  connectCombo(ui->comboIdlePulseDefault, ServoCv::IdlePulseDefault);
  connectCombo(ui->comboPowerWhenIdle, ServoCv::PowerWhenIdle);
  connectField(ui->fieldPowerStartUpDelay, ServoCv::PowerStartUpDelay);
  connectField(ui->fieldPulseOnBefore, ServoCv::PulseOnBefore);
  connectField(ui->fieldPulseOffAfter, ServoCv::PulseOffAfter);
  connectField(ui->fieldPowerOnBefore, ServoCv::PowerOnBefore);
  connectField(ui->fieldPowerOffAfter, ServoCv::PowerOffAfter);
  connect(ui->buttonServoCvsTabDefaults, &QPushButton::clicked, this, &ServoCvsTab::pushedDefaults);
  connect(ui->buttonServoCvsTabGet, &QPushButton::clicked, this, &ServoCvsTab::pushedGet);
  connect(ui->buttonServoCvsTabSet, &QPushButton::clicked, this, &ServoCvsTab::pushedSet);
}

ServoCvsTab::~ServoCvsTab()
{
  delete ui;
}

void ServoCvsTab::connectCombo(QComboBox *combo, int offset)
{
  // The CV value is the index of the chosen item (No/Yes, Low/High, Low/High/Pulses and the servo types)
  connect(combo, &QComboBox::activated, this, [this, offset](int index) {
    storeServoCv(offset, index);
    colorSetButton(true);
    updateTab();
  });
}

void ServoCvsTab::connectField(QSpinBox *field, int offset)
{
  // The field reacts on Return / leaving the field and on the arrows, not on every typed character
  field->setKeyboardTracking(false);
  connect(field, &QSpinBox::valueChanged, this, [this, offset](int value) {
    storeServoCv(offset, value);
    colorSetButton(true);
    updateTab();
  });
}

void ServoCvsTab::storeServoCv(int offset, int value)
{
  if (value < 0) value = 0;
  if (value > 255) value = 255;
  decoder->setCv(servoCv(offset), value);
}

void ServoCvsTab::storeCurve(int offset, QComboBox *combo, QCheckBox *check)
{
  // Bit 7: direction (1 = reverse). Bit 6: storage (1 = EEPROM). Bits 5..0: index (PROGMEM: 0..11)
  // CurveB: 0 = symmetric (CurveA in opposite direction). Symmetric + Reverse thus gives lin_A in reverse direction
  QString name = combo->currentText();                    // Empty if a value is shown that can not be chosen
  int index = curveNames.indexOf(name);
  int newValue;
  if (name == "Symmetric") index = 0;
  if (index >= 0) newValue = index;
    else newValue = decoder->getCv(servoCv(offset)) & 0x7F;   // EEPROM curve: only the direction changes
  if (check->isChecked()) newValue = newValue | 0x80;
  storeServoCv(offset, newValue);
  colorSetButton(true);
  updateTab();
}

void ServoCvsTab::showCurve(int offset, QComboBox *combo, QCheckBox *check)
{
  uint8_t value = decoder->getCv(servoCv(offset));
  int index = value & 0x3F;
  QString newString;
  if ((offset == ServoCv::CurveB) && ((value == 0) || (value == 255))) {
    showComboText(combo, "Symmetric");
    check->setChecked(false);
    return;
  }
  if (value & 0x40) newString = QString("EEPROM %1").arg(index);   // Can be shown, not chosen
  else if (index < 12) newString = curveNames[index];
  else newString = QString::number(value);
  showComboText(combo, newString);
  check->setChecked((value & 0x80) != 0);
}

void ServoCvsTab::pushedDefaults()
{
  // Defaults as in servo_CVs.cpp
  storeServoCv(ServoCv::CurveA, 2);                       // move_A
  storeServoCv(ServoCv::CurveB, 0);                       // symmetric
  storeServoCv(ServoCv::InvertServoDir, 0);
  storeServoCv(ServoCv::InvertRelais, 0);
  storeServoCv(ServoCv::ServoType, 0);                    // Generic
  storeServoCv(ServoCv::PulseStartUpValue, 1);            // High
  storeServoCv(ServoCv::PowerStartUpDelay, 5);            // 100 ms
  storeServoCv(ServoCv::IdlePulseDefault, 1);             // High
  storeServoCv(ServoCv::PulseOnBefore, 0);
  storeServoCv(ServoCv::PulseOffAfter, 10);               // 200 ms
  // PowerWhenIdle for ServoType Generic: Yes on the servo-3 board (with No, Generic and Uhlenbrock / Weinert
  // servos do not always work reliably on this board), No on the servo-2 and servo-6 boards
  if (decoder->getCv(CV::DecType) == 21) storeServoCv(ServoCv::PowerWhenIdle, 1);
    else storeServoCv(ServoCv::PowerWhenIdle, 0);
  storeServoCv(ServoCv::PowerOnBefore, 0);
  storeServoCv(ServoCv::PowerOffAfter, 10);               // 200 ms
  colorSetButton(true);
  updateTab();
}

void ServoCvsTab::pushedGet()
{
  mainWindow->readServoCvs();
  colorSetButton(false);
}

void ServoCvsTab::pushedSet()
{
  pom->writeCv(servoCv(ServoCv::CurveA));
  pom->writeCv(servoCv(ServoCv::CurveB));
  for (int offset = ServoCv::InvertServoDir; offset <= ServoCv::PowerOffAfter; offset++) pom->writeCv(servoCv(offset));
  colorSetButton(false);
}

void ServoCvsTab::updateTab()
{
  fillServoPopup(ui->popupServoCvs);
  // Curves
  showCurve(ServoCv::CurveA, ui->comboCurveA, ui->checkCurveAReverse);
  showCurve(ServoCv::CurveB, ui->comboCurveB, ui->checkCurveBReverse);
  // The combo boxes: the CV value is the index of the item. For No/Yes and Low/High every value above 0 is shown
  // as the second item (as in the macOS version)
  QComboBox *combos[] = {ui->comboInvertServoDir, ui->comboInvertRelais, ui->comboServoType,
                         ui->comboPulseStartUpValue, ui->comboIdlePulseDefault, ui->comboPowerWhenIdle};
  int offsets[] = {ServoCv::InvertServoDir, ServoCv::InvertRelais, ServoCv::ServoType,
                   ServoCv::PulseStartUpValue, ServoCv::IdlePulseDefault, ServoCv::PowerWhenIdle};
  for (int i = 0; i < 6; i++) {
    int value = decoder->getCv(servoCv(offsets[i]));
    if ((combos[i]->count() == 2) && (value > 1)) value = 1;
    if (value < combos[i]->count()) combos[i]->setCurrentIndex(value);
      else showComboText(combos[i], QString::number(value));
  }
  ui->comboInvertRelais->setEnabled(decoder->getCv(CV::DecType) == 20);   // Only the servo-2 board has relays
  // The fields are set without calling storeServoCv
  QSpinBox *fields[] = {ui->fieldPowerStartUpDelay, ui->fieldPulseOnBefore, ui->fieldPulseOffAfter,
                        ui->fieldPowerOnBefore, ui->fieldPowerOffAfter};
  int fieldOffsets[] = {ServoCv::PowerStartUpDelay, ServoCv::PulseOnBefore, ServoCv::PulseOffAfter,
                        ServoCv::PowerOnBefore, ServoCv::PowerOffAfter};
  for (int i = 0; i < 5; i++) {
    QSignalBlocker block(fields[i]);
    fields[i]->setValue(decoder->getCv(servoCv(fieldOffsets[i])));
  }
  // Offset 10..14, 16 and 17 are only used with ServoType 0 (Generic); for the other types the firmware
  // uses fixed values. Offset 16 and 17 have no effect if PowerWhenIdle = 1 (power is not switched).
  bool generic = (decoder->getCv(servoCv(ServoCv::ServoType)) == 0);
  bool powerSwitched = (generic && (decoder->getCv(servoCv(ServoCv::PowerWhenIdle)) == 0));
  ui->comboPulseStartUpValue->setEnabled(generic);
  ui->fieldPowerStartUpDelay->setEnabled(generic);
  ui->comboIdlePulseDefault->setEnabled(generic);
  ui->fieldPulseOnBefore->setEnabled(generic);
  ui->fieldPulseOffAfter->setEnabled(generic);
  ui->fieldPowerOnBefore->setEnabled(powerSwitched);
  ui->fieldPowerOffAfter->setEnabled(powerSwitched);
}

void ServoCvsTab::colorSetButton(bool red)
{
  setButtonRed(ui->buttonServoCvsTabSet, red);
}
