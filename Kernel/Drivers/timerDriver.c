#include "timerDriver.h"
#include "consoleDriver.h"
#include "../Graphics/gui.h"

static unsigned int tickCounter = 0;
static unsigned int rseed = 2463534242;

void timerISR(void)
{
    tickCounter++;
    tick();

    if(!isGUIInitialized)
    {
        updateCursor();
    }
    else if (tickCounter % 100 == 0) //update time every second
    {
        drawTimeLabel(getRTCTime());
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

unsigned int getTicks()
{
    return tickCounter;
}

void sleep(unsigned int ms)
{
    if (ms == 0) return;
    unsigned int ticks = (ms + 9) / 10;  // round UP so sleep(1) waits 1 tick not 0
    unsigned int targetTicks = tickCounter + ticks;
    while (tickCounter < targetTicks) asm volatile ("sti; hlt");
}
unsigned int getSeconds(void)
{
    return tickCounter / 100;
}

void initializeRTC(void)
{
    unsigned char prev = 0;
    outb(REAL_TIME_PORT, 0x8B);
    prev = inb(REAL_TIME_DATA_PORT);
    outb(REAL_TIME_PORT, 0x8B);
    outb(REAL_TIME_DATA_PORT, prev | 0x40);
    outb(REAL_TIME_PORT, 0x0C);
    inb(REAL_TIME_DATA_PORT);
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
    t.hours = bcdToDec(readRTC(HOUR_REG)); 
    t.minutes = bcdToDec(readRTC(MINUTE_REG)); 
    t.seconds = bcdToDec(readRTC(SECOND_REG));

    t.ms = 0;
    return t;
}
//binary time to decimal in short
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
    //quick fix for the missing 0 bug
    if(currentTime.hours < 10)
    {
        print("0", WHITE);
    }
    printNumber(currentTime.hours + 3, WHITE);
    print(":", WHITE);
    if(currentTime.minutes < 10)
    {
        print("0", WHITE);
    }
    printNumber(currentTime.minutes, WHITE);
    print(":", WHITE);
    if(currentTime.seconds < 10)
    {
        print("0", WHITE);
    }
    printNumber(currentTime.seconds, WHITE);
    printLine("", WHITE);
}

void srand()
{
    rseed = tickCounter; //seed with current tick count for sort of randomness i guess...
}

unsigned int rand()
{
    rseed ^= rseed << 13;
    rseed ^= rseed >> 17;
    rseed ^= rseed << 5;
    return rseed;
}

unsigned int randRange(unsigned int min, unsigned int max)
{
    if (max <= min) return min;
    unsigned int range = max - min + 1;
    return (rand() % range) + min;
}