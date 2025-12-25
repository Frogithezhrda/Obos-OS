#ifndef ERRORS_H
#define ERRORS_H

#include "../Drivers/consoleDriver.h"

#define EXCEPTION_COUNT 32

void exceptionHandler(const char id);

void kernelPanic(const char* kernel);

#endif