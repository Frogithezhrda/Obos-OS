#ifndef KEYBOARD_DRIVER_H
#define KEYBOARD_DRIVER_H

#include "consoleDriver.h"
#include "../Tables/PIC.h"

#define KEYBOARD_BUFFER_SIZE 256


void keyboardISR(void);
void deleteChar(void);
void initializeKeyboard(void);

#endif