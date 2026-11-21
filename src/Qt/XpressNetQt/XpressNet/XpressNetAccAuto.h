//
//  XpressNetAccAuto.h
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of the AccAutoClass of the VB6 XpressNet DLL. See XpressNetAccAuto.cpp
//  For internal use by the library.
//

#ifndef XPRESSNETACCAUTO_H
#define XPRESSNETACCAUTO_H

#include "XpressNet.h"
#include <QQueue>
#include <QTimer>

class AccAutoClass
{
public:
  explicit AccAutoClass(XpressNetClass *parent);
  ~AccAutoClass();                                        // Stops the timer

  void handle(int address, quint8 position);
  void resume();                                          // 2.0
  int size();
  int emptyFifo();                                        // 2.0: returns the number of removed commands

private:
  XpressNetClass *Parent;

  // We store accessory commands in a FIFO. Each element holds two fields: the address and the position of the switch.
  // The maximum number of commands is set by maxFifoEntries.
  static const int maxFifoEntries = 32;                   // The number of Accessory messages the queue is able to hold
  struct AccessoryDataType
  {
    int address;
    quint8 position;
  };
  QQueue<AccessoryDataType> fifo;                         // this is the queue
  bool pausing;                                           // is the timer used for the pause after a deactivation?
  bool switchActivated;                                   // 2.0: is the output of the switch at the head of the FIFO active?
  bool waitingForInterface;                               // 2.0: does an activation wait for the end of a pause (01 07)?
  QTimer deactTimer;

  void activateHead();                                    // 2.0
  void deactTimer_Timer();
};

#endif // XPRESSNETACCAUTO_H
