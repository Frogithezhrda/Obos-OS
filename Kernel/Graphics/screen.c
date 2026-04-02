#include "screen.h"

void printPixel(Pixel pixel)
{
    unsigned char* fb = (unsigned char*)FB_BASE;
    if (pixel.x >= SCREEN_WIDTH || pixel.y >= SCREEN_HEIGHT)
    {
        return;
    }
    unsigned int offset = OFFSET(pixel);
    fb[offset]     = pixel.color.r;  // R
    fb[offset + 1] = pixel.color.g;  // G
    fb[offset + 2] = pixel.color.b;  // B
}
// void clearScreen()
// {

// }

// void printChar(char c, unsigned char color)
// {

// }