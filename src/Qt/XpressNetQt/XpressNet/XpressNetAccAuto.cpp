//
//  XpressNetAccAuto.cpp
//  XpressNetQt
//
//  Created by Aiko Pras on 05-11-2026.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  Qt version of the AccAutoClass of the VB6 XpressNet DLL: implementation of CmdAccessoryAuto.
//  A port of XpressNetAccAuto.cs of the .NET version.
//
//  This method can be used to set multiple switches at once, without worrying about deactivation
//  and scheduling. The standard XpressNet command for accesories include an activation and deactivation
//  bit, which allows activation of the decoder's output port and, after a short interval, deactivation.
//  Such deactivation may, for example, be necessary for coil or motor based switch engines.
//  In practice, coild based switch engines need to be deactivated after several hundreds of ms.
//  Another concern with switch decoders is that, after activation, a single output may consume 1 A
//  of current. If all switches would activate their outputs at exactly the same moment, the amount
//  of current that is needed may easily exceed the amount that is available. Therefore it is importent
//  to schedule switch commands after each other.
//
//  For the interval between activation and deactivation the "AccessoryInterval" variable (declared in the
//  XpressNet class) will be used. After a deactivation there is a pause of AccessoryInterval / 2, before the
//  next accessory is activated (as in the example of the Z21 specification, 5.2.1).
//
//  Like the "message" subroutine, this method implements a FIFO queue to store accessory commands
//  As soon as an accessory command is received, the accessory address and position is saved in the FIFO.
//  Next, the associated XpressNet accessory command, with the activation bit set, is issued, and a
//  timer is started. Once the timer fires, the XpressNet command is send again, but this time the
//  activation bit is released.
//  If, in the time between activation and deactivation, additional accessory commands are received,
//  these are stored on the FIFO queue as well.
//
//  This method is implemented in a separate class, since it is, strictly speaking, not part of the
//  XpressNet interface, but rather a layer on top of the XpressNet library. Still it should be instantiated
//  in, and called from the XpressNet class object.
//
//  Version 2.0 (November 2026), the same in the VB6, .NET, macOS and Qt versions:
//  - emptyFifo (after a response timeout, or after a pause without 01 07) first switches off the output of the
//    switch that is activated at that moment. In version 1.3 this switch got no deactivation.
//  - During a pause (see XpressNetSend.cpp: the command station does not address the interface) no new switch is
//    activated. The activation of the next switch waits for 01 07 (resume).
//  - A full queue gives the event SendError (besides the line in the logfile).
//  - The interval of AccessoryInterval still starts at the moment the activation is offered to the SendClass
//    (as in version 1.3). If the activation waits in the FIFO of the SendClass, the output is active shorter.
//

#include "XpressNetAccAuto.h"
#include "XpressNetSend.h"
#include "XpressNetLogFile.h"


AccAutoClass::AccAutoClass(XpressNetClass *parent)
{
  Parent = parent;
  switchActivated = false;
  waitingForInterface = false;
  pausing = false;
  // The timer fires once; it is started again for every activation, deactivation and pause. The timer is precise
  // (Qt::PreciseTimer): a coarse timer (the default) may be up to 5% early or late
  deactTimer.setSingleShot(true);
  deactTimer.setTimerType(Qt::PreciseTimer);
  QObject::connect(&deactTimer, &QTimer::timeout, &deactTimer, [this]() { deactTimer_Timer(); });
  emptyFifo();
}

// Stops the timer
AccAutoClass::~AccAutoClass()
{
  deactTimer.stop();
}

void AccAutoClass::handle(int address, quint8 position)
{
  // Check if the FIFO queue has free slots left to store the message
  // If not, increase "maxFifoEntries", or modify the calling program
  if (fifo.size() >= maxFifoEntries) {                    // FIFO overflow :-(
    Parent->Logfile->Errors("Error: Auto Accessory queue is full");
    // 2.0: event SendError
    Parent->eventSendError(FifoVoll, "Error: Auto Accessory queue is full (CmdAccessoryAuto: switch " + QString::number(address) + ", position " + QString::number(position) + ")");
    return;
  }
  AccessoryDataType accessory;                            // store address and position to the FIFO
  accessory.address = address;
  accessory.position = position;
  fifo.enqueue(accessory);
  // If this is the only accessory command in the FIFO, we may send it right now
  // But not during the pause after a deactivation (the accessory is activated after the pause)
  if ((fifo.size() == 1) && (!pausing)) {
    activateHead();                                       // activate the accessory, and start the timer
  }
}

// 2.0: activates the output of the switch at the head of the FIFO, and starts the deactivation timer
// (1.3: the same code in handle and deactTimer_Timer). Not during a pause of the SendClass (01 05 / 01 08):
// the activation then waits for resume (01 07)
void AccAutoClass::activateHead()
{
  if (Parent->send->interfaceNotAddressed) {
    waitingForInterface = true;
    return;
  }
  AccessoryDataType head = fifo.head();
  Parent->CmdAccessory(head.address, head.position, 1);   // activate the accessory
  switchActivated = true;
  deactTimer.start(Parent->AccessoryInterval);            // start the deactivation timer
}

void AccAutoClass::deactTimer_Timer()
{
  // STEP 1: If the timer expires, we need to send the accessory command to deactivate the output port
  // If the pause after a deactivation is over (see STEP 3), activate the port of the next
  // accessory (if the FIFO is not empty), and start the timer
  if (pausing) {
    pausing = false;
    if (fifo.size() > 0) activateHead();
    return;
  }
  if (fifo.size() == 0) return;                           // should not occur: the timer stops if the FIFO is emptied
  AccessoryDataType head = fifo.head();
  switchActivated = false;
  Parent->CmdAccessory(head.address, head.position, 0);
  // STEP 2: After deactivation, we should remove this accesory from the FIFO
  if (fifo.size() > 0) fifo.dequeue();
  // STEP 3: First a pause of AccessoryInterval / 2. The next accessory is activated after the pause
  pausing = true;
  deactTimer.start(Parent->AccessoryInterval / 2);        // start the timer for the pause
}

// 2.0: called by the SendClass at the end of a pause (01 07). An activation that waited, is done now
void AccAutoClass::resume()
{
  if (!waitingForInterface) return;
  waitingForInterface = false;
  if ((fifo.size() > 0) && (!pausing) && (!switchActivated)) activateHead();
}

int AccAutoClass::size()
{
  return fifo.size();
}

// 2.0: the output of an activated switch is switched off first (1.3: this switch got no deactivation).
// Returns the number of removed commands (without the switch that has been switched off)
int AccAutoClass::emptyFifo()
{
  int removed = fifo.size();
  if (switchActivated && (fifo.size() > 0)) {
    switchActivated = false;
    AccessoryDataType head = fifo.head();
    Parent->CmdAccessory(head.address, head.position, 0);
    removed = removed - 1;
  }
  switchActivated = false;
  waitingForInterface = false;
  fifo.clear();
  // Stop the deactivation timer as well
  deactTimer.stop();
  pausing = false;
  return removed;
}
