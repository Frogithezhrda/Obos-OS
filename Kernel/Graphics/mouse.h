#ifndef MOUSE_H
#define MOUSE_H

#include "../Tables/PIC.h"
#include "../Drivers/consoleDriver.h"
#include "gui.h"
void mouseIRQHandler();
void mouseInit();
void redrawMouse();
void eraseMouse();

extern volatile int mouseErased;

#endif