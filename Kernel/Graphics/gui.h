#ifndef GUI_H
#define GUI_H

// #include "../Drivers/consoleDriver.h"
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
    char isVisible;
} Window;

typedef struct Label
{
    Window window;
    char* text;
    unsigned int size;
    Color textColor;
} Label;

typedef struct Button
{
    Label label;
    void (*onClick)();
} Button;

typedef struct TextBox
{
    Label label;
    unsigned int maxSize;
    unsigned int cursorPos;
    void (*onEnter)(struct TextBox* tb);
} TextBox;

typedef struct Icon
{
    Window window;
    unsigned char (*iconData)[16]; //2D array to store the icon's pixel data (1 byte per pixel, 0 for transparent, 1 for foreground color)
    void (*onClick)();
} Icon;


void drawButton(Button* button);
void drawIcon(Icon* icon, unsigned int scale);
void drawLabel(Label* label);
void drawWindow(Window* win);
void drawTextBox(TextBox* textBox);
void drawPixel(Pixel p);

Window createWindow(unsigned int x, unsigned int y, unsigned int width, unsigned int height, Color color, char isVisible);
Icon createIcon(Window* window, unsigned char (*iconData)[16], void (*onClick)());
Label createLabel(Window* window, char* text, unsigned int size, Color color);
Button createButton(Label* label, void (*onClick)());
TextBox createTextBox(Label* label, unsigned int size);

void initalizeWindowGUI();
void handleClick(unsigned int mouseX, unsigned int mouseY);
void textBoxHandleKey();
void setFocusedTextBox(TextBox* txtBox);
void eraseWindow();


extern TextBox* focusedTextBox;
extern unsigned int isGUIInitialized;
extern Button exitButton;
#endif