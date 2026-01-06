#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

#include "../Tables/PIC.h"
#include "consoleDriver.h"

#define BASE_FREQUENCY 1193180
#define COMMAND_PORT 0x43
#define CHANNEL_0_PORT 0x40

#define REAL_TIME_PORT 0x70
#define REAL_TIME_DATA_PORT 0x71
#define SECOND_REG 0x00
#define MINUTE_REG 0x02
#define HOUR_REG 0x04 

typedef struct Time
{
    unsigned int hours;
    unsigned int minutes;
    unsigned int seconds;
    unsigned int ms;
} Time;

void timerISR(void);
void initializeTimer(void);
void sleep(unsigned int ms);
unsigned int getSeconds(void);
void initializeRTC(void);
void rtcISR(void);
Time getRTCTime(void);
unsigned char readRTC(unsigned char realTimeReg);
void printCurrentTime(void);
unsigned char bcdToDec(unsigned char bcd);

#endif