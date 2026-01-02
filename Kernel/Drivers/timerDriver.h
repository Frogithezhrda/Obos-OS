#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

#include "../Tables/PIC.h"
#include "consoleDriver.h"

void timerISR(void);
void initializeTimer(void);

#endif