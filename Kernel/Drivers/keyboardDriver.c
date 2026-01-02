#include "keyboardDriver.h"


void keyboardISR(void)
{
    endOfInterrupt(1); //IRQ1 is keyboard
}

void initializeKeyboard(void)
{
    //**TODO**//
}