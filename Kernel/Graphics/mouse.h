#ifndef MOUSE_H
#define MOUSE_H

#include "../Tables/PIC.h"
#include "../Drivers/consoleDriver.h"
#include "gui.h"
void mouseIRQHandler();
void mouseInit();
void redrawMouse();
void eraseMouse();
void drawMouse();

extern volatile int mouseErased;
extern volatile int mouseLeft;
extern int mouseX;
extern int mouseY;

#endif