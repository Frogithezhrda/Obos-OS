#ifndef ERRORS_H
#define ERRORS_H

#include "../Drivers/consoleDriver.h"

#define EXCEPTION_COUNT 32

void exceptionHandler(const unsigned char id);

void kernelPanic(const char* kernel);

#endif