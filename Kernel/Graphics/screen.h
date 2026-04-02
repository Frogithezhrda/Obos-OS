#ifndef SCREEN_H
#define SCREEN_H

#define FB_BASE 0xFD000000
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768


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

#define OFFSET(pixel) (pixel.y * SCREEN_WIDTH + pixel.x) * 3;

void printPixel(Pixel p);
// void clearScreen();
// void printChar(char c, unsigned char color);

#endif