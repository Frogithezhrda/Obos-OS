#ifndef WM_H
#define WM_H
#include "../Graphics/gui.h"
#include "../Graphics/mouse.h"

#define FRAME_TICKS 2

typedef void (*DrawFn)(int x, int y, int w, int h);
typedef void (*ClickFn)(int lx, int ly);

void wmSetClick(int id, ClickFn fn);
int wmAddWindow(int x, int y, int w, int h, const char* title, Color bg, DrawFn draw);
void wmUpdate();
void wmDraw();
const char* wmTitle(int index);
int wmIsFocused(int index);
void wmFocus(int index);
int wmNextId(void);
int wmIndexOfId(int id);
int wmHitTest(int px, int py);
int wmConsumedClick(void);
void wmCloseId(int id);
typedef void (*KeyFn)(int key);
void wmSetKey(int id, KeyFn fn);
void wmKey(int key);

#endif