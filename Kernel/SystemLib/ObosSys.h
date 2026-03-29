#ifndef OBOSSYS_H
#define OBOSSYS_H


/*
a lib for syscalls a wrapper
*/
#include "../Drivers/consoleDriver.h"
#include "../Drivers/timerDriver.h"

void printo(const char* string, unsigned int color);
unsigned int getObosTicks(void);
void* malloc(unsigned int size);
void free(void* ptr);
unsigned int getObosTime(Time* timePtr);
void sleepo(unsigned int ms);
unsigned int createFileo(const char* filename);
unsigned int deleteFileo(const char* filename);
unsigned int createDiro(const char* dirname);
unsigned int readFileo(const char* filename, char* buffer, unsigned int size);
unsigned int writeFileo(const char* filename, const char* data, unsigned int size);
unsigned int rando();

#define printoW(string) printo(string, 0)

#endif