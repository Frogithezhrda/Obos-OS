#include "timerDriver.h"

static unsigned int tickCounter = 0;

void timerISR(void)
{
    tickCounter++;
    if (tickCounter % 100 == 0) 
    {
        print(".", WHITE);
    }
    endOfInterrupt(0); //IRQ0 is timer
}

void initializeTimer(void)
{
    unsigned int frequency = 100;
    unsigned int divisor = BASE_FREQUENCY / frequency;
    outb(COMMAND_PORT, 0x36); 
    outb(CHANNEL_0_PORT, (unsigned char)(divisor & 0xFF)); 
    outb(CHANNEL_0_PORT, (unsigned char)((divisor >> 8) & 0xFF)); 
}