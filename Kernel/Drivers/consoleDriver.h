#ifndef CONSOLEDRIVER_H
#define CONSOLEDRIVER_H

#define VGA_COMMAND_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5
#define VIDEO_LOCATION 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

enum VGAColor 
{
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    BROWN = 6,
    LIGHT_GREY = 7,
    DARK_GREY = 8,
    LIGHT_BLUE = 9,
    LIGHT_GREEN = 10,
    LIGHT_CYAN = 11,
    LIGHT_RED = 12,
    LIGHT_MAGENTA = 13,
    LIGHT_BROWN = 14,
    WHITE = 15
};

void clearScreen();
void print(const char* string, const int color);
void printChar(const char character, const int color);
void printLine(const char* string, const int color);
void printNumber(int number, const int color);
void updateCursor(const int cursorPos);

#endif