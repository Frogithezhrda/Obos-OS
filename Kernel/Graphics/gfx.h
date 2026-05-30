#ifndef GFX_H
#define GFX_H

#include "screen.h"
#include "../Memory/heap.h"
#include "../SystemLib/obosMemory.h"
#include "icons.h"


void gfxInit();
void gfxFlush();
void gfxPutPixel(unsigned int x, unsigned int y, Color color);
void gfxFillRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height, Color color);
void gfxClear(Color color);
void gfxDrawIcon(unsigned int x, unsigned int y, unsigned char data[16][16], unsigned int scale);
void gfxDrawChar(unsigned int x, unsigned int y, char character, Color color);
void gfxDrawString(const char* str, unsigned int x, unsigned int y, Color color);

#endif // GFX_H