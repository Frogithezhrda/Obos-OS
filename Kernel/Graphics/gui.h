#ifndef GUI_H
#define GUI_H

#include "../Drivers/consoleDriver.h"
#include "icons.h"
#include "../Drivers/timerDriver.h"

#define VISIBLE 1
#define INVISIBLE 0

#define GUI_FONT_SCALE 2

typedef struct Window
{
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
    Color bgColor;
    int isVisible;
} Window;

typedef struct Button
{
    Window window;
    char* text;
    Color textColor;
    void (*onClick)();
} Button;

typedef struct Icon
{
    Window window;
    unsigned char (*iconData)[16]; //2D array to store the icon's pixel data (1 byte per pixel, 0 for transparent, 1 for foreground color)
} Icon;

typedef struct Label
{
    Window window;
    char* text;
    Color textColor;
} Label;

void drawButton(Button* button);
void drawIcon(Icon* icon);
void drawLabel(Label* label);
void drawWindow(Window* win);
void initalizeWindowGUI();

extern unsigned int isGUIInitialized;

#endif