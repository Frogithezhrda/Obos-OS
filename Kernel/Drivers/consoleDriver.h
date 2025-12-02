#ifndef CONSOLEDRIVER_H
#define CONSOLEDRIVER_H

#define VIDEO_LOCATION 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

void clearScreen();
void print(const char* string);

#endif