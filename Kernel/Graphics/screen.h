#ifndef SCREEN_H
#define SCREEN_H

#include "fonts.h"

#define FB_BASE 0xFD000000
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

#define FONT_SCALE 1

typedef struct Color
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Color;

typedef struct Pixel
{
    unsigned int x;
    unsigned int y;
    Color color;
} Pixel;

#define OFFSET(pixel) ((pixel.y * SCREEN_WIDTH + pixel.x) * 3)

void printPixel(Pixel p);
Color getPixelColor(unsigned int x, unsigned int y);
void printCharVBE(char c, unsigned int x, unsigned int y, Color color);
void printStringVBE(const char* str, unsigned int x, unsigned int y, Color color);
#endif