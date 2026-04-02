#include "screen.h"

void printPixel(Pixel pixel)
{
    unsigned char* fb = (unsigned char*)FB_BASE;
    if (pixel.x >= SCREEN_WIDTH || pixel.y >= SCREEN_HEIGHT)
    {
        return;
    }
    unsigned int offset = OFFSET(pixel);
    fb[offset] = pixel.color.r;  // R
    fb[offset + 1] = pixel.color.g;  // G
    fb[offset + 2] = pixel.color.b;  // B
}
void printCharVBE(char c, unsigned int x, unsigned int y, Color color)
{
    unsigned char* glyph = font[(unsigned char)c];
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
                        Pixel p = {x + col * FONT_SCALE + sx, y + row * FONT_SCALE + sy, color};
                        printPixel(p);
                    }
                }
            }
        }
    }
}

void printStringVBE(const char* str, unsigned int x, unsigned int y, Color color)
{
    unsigned int origX = x;
    for (unsigned int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\n')
        {
            y += 8 * FONT_SCALE;
            x = origX;
        }
        else
        {
            printCharVBE(str[i], x, y, color);
            x += 8 * FONT_SCALE;
        }
    }
}