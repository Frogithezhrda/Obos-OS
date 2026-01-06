#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

#include "../Tables/PIC.h"
#include "consoleDriver.h"

#define BASE_FREQUENCY 1193180
#define COMMAND_PORT 0x43
#define CHANNEL_0_PORT 0x40
void timerISR(void);
void initializeTimer(void);

#endif