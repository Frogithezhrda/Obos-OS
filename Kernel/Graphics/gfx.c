#include "gfx.h"

unsigned char* backBuffer;

static void (*yieldHook)(void) = 0;

void gfxSetYield(void (*fn)(void))
{
    yieldHook = fn;
}

void gfxInit()
{
    backBuffer = (unsigned char*)kmalloc(SCREEN_WIDTH * SCREEN_HEIGHT * 3);
    memset(backBuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * 3);
}

void gfxPutPixel(unsigned int x, unsigned int y, Color color)
{
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;

    unsigned int offset = (y * SCREEN_WIDTH + x) * 3;
    backBuffer[offset] = color.b;
    backBuffer[offset + 1] = color.g;
    backBuffer[offset + 2] = color.r;
    // backBuffer[offset + 3] = 255; // alpha
}

void gfxFillRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height, Color color)
{
    for (unsigned int j = 0; j < height; j++)
    {
        if (yieldHook) yieldHook();
        for (unsigned int i = 0; i < width; i++)
        {
            gfxPutPixel(x + i, y + j, color);
        }
    }
}

void gfxClear(Color color)
{
    gfxFillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
}

void gfxDrawIcon(unsigned int x, unsigned int y, unsigned char data[16][16], unsigned int scale)
{
    for (int row = 0; row < 16; row++)
        for (int col = 0; col < 16; col++)
        {
            if (data[row][col] == 0) continue;
            Color color = palette[data[row][col]];
            gfxFillRect(x + col * scale, y + row * scale, scale, scale, color);
        }
}

void gfxDrawChar(unsigned int x, unsigned int y, char character, Color color)
{
    unsigned char* glyph = font[(unsigned char)character];
    for (unsigned int row = 0; row < 8; row++)
    {
        for (unsigned int col = 0; col < 8; col++)
        {
            if (glyph[row] & (0x80 >> col))
            {
                for (unsigned int sy = 0; sy < FONT_SCALE; sy++)
                {
                    for (unsigned int sx = 0; sx < FONT_SCALE; sx++)
                    {
                        gfxPutPixel(x + col * FONT_SCALE + sx, y + row * FONT_SCALE + sy, color);
                    }
                }
            }
        }
    }
}


void gfxDrawString(const char* str, unsigned int x, unsigned int y, Color color)
{
    unsigned int origX = x;
    for (unsigned int i = 0; str[i] != '\0'; i++)
    {
        if (yieldHook) yieldHook();
        if (str[i] == '\n')
        {
            y += 8 * FONT_SCALE;
            x = origX;
        }
        else
        {
            gfxDrawChar(x, y, str[i], color);
            x += 8 * FONT_SCALE;
        }
    }
}

void gfxFlush()
{
    unsigned int total = SCREEN_WIDTH * SCREEN_HEIGHT * 3;
    unsigned int chunk = SCREEN_WIDTH * 3 * 16;
    for (unsigned int off = 0; off < total; off += chunk)
    {
        unsigned int n = (total - off < chunk) ? (total - off) : chunk;
        memcpy((unsigned char*)FB_BASE + off, backBuffer + off, n);
        if (yieldHook) yieldHook();
    }
}