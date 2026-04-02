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
// void clearScreen()
// {

// }

void printCharVBE(char c, unsigned int x, unsigned int y, Color color)
{
    unsigned char* glyph = font[(unsigned char)c];
    for (unsigned int row = 0; row < 8; row++)
    {
        for (unsigned int col = 0; col < 8; col++)
        {
            if (glyph[row] & (0x80 >> col))
            {
                Pixel p = {x + col * FONT_SCALE, y + row, color};
                printPixel(p);
            }
        }
    }
}