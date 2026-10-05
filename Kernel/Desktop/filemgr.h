#ifndef FILEMGR_H
#define FILEMGR_H

#include "../SystemLib/util.h"

void filemgrDraw(int x, int y, int w, int h);
void filemgrClick(int lx, int ly);
void filemgrKey(int key);
void filemgrRefresh(void);
#endif