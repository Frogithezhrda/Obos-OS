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

void sleep(unsigned int ms)
{
    unsigned int targetTicks = tickCounter + (ms / 10);
    //hlt until target ticks reached
    while (tickCounter < targetTicks) asm volatile ("hlt");
}
unsigned int getSeconds(void)
{
    return tickCounter / 100;
}

void initializeRTC(void)
{
    unsigned char prev = 0;
    outb(0x70, 0x8B);
    prev = inb(0x71);
    outb(0x70, 0x8B);
    outb(0x71, prev | 0x40);
    outb(0x70, 0x0C);
    inb(0x71);
}

unsigned char readRTC(unsigned char realTimeReg)
{
    unsigned int value = 0;
    outb(REAL_TIME_PORT, realTimeReg);
    value = inb(REAL_TIME_DATA_PORT);
    return value;
}

void rtcISR(void)
{
    outb(0x70, 0x0C);
    inb(0x71);
    endOfInterrupt(8); // send EOI to PIC
}
Time getRTCTime(void)
{
    Time t;
    t.hours   = bcdToDec(readRTC(HOUR_REG)); 
    t.minutes = bcdToDec(readRTC(MINUTE_REG)); 
    t.seconds = bcdToDec(readRTC(SECOND_REG));

    t.ms = 0;
    return t;
}

unsigned char bcdToDec(unsigned char bcd)
{
    unsigned char tens  = (bcd >> 4) & 0x0F; 
    unsigned char units = bcd & 0x0F;        
    return tens * 10 + units;
}

void printCurrentTime(void)
{
    Time currentTime = getRTCTime();
    print("Current Time: ", WHITE);
    printNumber(currentTime.hours, WHITE);
    print(":", WHITE);
    printNumber(currentTime.minutes, WHITE);
    print(":", WHITE);
    printNumber(currentTime.seconds, WHITE);
    printLine("", WHITE);
}