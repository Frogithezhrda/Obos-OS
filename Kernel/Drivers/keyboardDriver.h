#ifndef KEYBOARD_DRIVER_H
#define KEYBOARD_DRIVER_H

#include "consoleDriver.h"
#include "../Tables/PIC.h"
#include "../SystemLib/obosMemory.h"
#include "../Graphics/gui.h"
#include "keys.h"

#define KEYBOARD_BUFFER_SIZE 256
#define KEYBOARD_SCAN_CODE_PORT 0x60
#define KEYPRESS_MASK 0b10000000
#define SHIFT_LEFT_PRESS 42
#define SHIFT_RIGHT_PRESS 54
#define SHIFT_LEFT_RELEASE 170
#define SHIFT_RIGHT_RELEASE 182
#define LAST_SCAN_CODE 84
#define KEY_QUEUE_SIZE 64

void keyboardISR(void);
void deleteChar(void);
void keybos(char* string, const int maxLength);
void keybosIndex(char* string, const int maxLength, int index);
char keybosChar();
void handleKeyRelease(void);
void keyboardPump(void);
int keyboardPoll(void);
// void keybosGUI(TextBox* tb);

#endif