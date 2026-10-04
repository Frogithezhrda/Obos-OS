#ifndef WM_H
#define WM_H
#include "../Graphics/gui.h"
#include "../Graphics/mouse.h"

#define TIMER_HZ 100
#define FRAME_TICKS 2

void wmAddWindow(int x, int y, int w, int h, const char* title, Color bg);
void wmUpdate();
void wmDraw();
const char* wmTitle(int index);
int wmIsFocused(int index);
void wmFocus(int index);
int wmNextId(void);
int wmIndexOfId(int id);

#endif