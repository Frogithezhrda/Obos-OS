#include "timerDriver.h"

static unsigned int tickCounter = 0;

void timerISR(void)
{
    tickCounter++;
    if (tickCounter % 100 == 0) 
    {
        print(".");
    }
    endOfInterrupt(0); //IRQ0 is timer
}

void initializeTimer(void)
{
    //**TODO**//
}