#ifndef CONSOLEDRIVER_H
#define CONSOLEDRIVER_H

// #define VGA_COMMAND_PORT 0x3D4
// #define VGA_DATA_PORT 0x3D5
// #define VIDEO_LOCATION 0xB8000
// #define VIDEO_LOCATION 0xC00B8000
#include "../Graphics/screen.h"
// #define VGA_VIRTUAL_ADDRESS 0xC00B8000

//can print now using White without writing the color each time
#define printW(s) print(s, WHITE)
#define printLineW(s) printLine(s, WHITE)
#define printCharW(c) printChar(c, WHITE)
#define printNumberW(n) printNumber(n, WHITE)
#define printHexW(n) printHexNumber(n, WHITE)

#define CHAR_W (8 * FONT_SCALE)
#define CHAR_H (8 * FONT_SCALE)

#define COLS (SCREEN_WIDTH  / CHAR_W)
#define ROWS (SCREEN_HEIGHT / (CHAR_H + 4))
// enum VGAColor 
// {
//     BLACK = 0,
//     BLUE = 1,
//     GREEN = 2,
//     CYAN = 3,
//     RED = 4,
//     MAGENTA = 5,
//     BROWN = 6,
//     LIGHT_GREY = 7,
//     DARK_GREY = 8,
//     LIGHT_BLUE = 9,
//     LIGHT_GREEN = 10,
//     LIGHT_CYAN = 11,
//     LIGHT_RED = 12,
//     LIGHT_MAGENTA = 13,
//     YELLOW = 14,
//     WHITE = 15
// };

extern const Color BLACK;
extern const Color BLUE;
extern const Color GREEN;
extern const Color CYAN;
extern const Color RED;
extern const Color MAGENTA;
extern const Color BROWN;
extern const Color LIGHT_GREY;
extern const Color DARK_GREY;
extern const Color LIGHT_BLUE;
extern const Color LIGHT_GREEN;
extern const Color LIGHT_CYAN;
extern const Color LIGHT_RED;
extern const Color LIGHT_MAGENTA;
extern const Color YELLOW;
extern const Color WHITE;

void clearScreen();
void print(const char* string, const Color color);
void printChar(const char character, const Color color);
void printLine(const char* string, const Color color);
void printNumber(int number, const Color color);
void printHexNumber(unsigned int number, const Color color);
void updateCursor();
// void scrollIfNeeded();

#endif