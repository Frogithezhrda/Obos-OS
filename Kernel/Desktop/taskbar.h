#ifndef TASKBAR_H
#define TASKBAR_H

#include "../Graphics/gfx.h"
#include "../Drivers/timerDriver.h"
#define TASKBAR_H_PX 28
#define TASKBAR_Y (SCREEN_HEIGHT - TASKBAR_H_PX)

void taskbarUpdate(void);
void taskbarDraw(void);

#endif