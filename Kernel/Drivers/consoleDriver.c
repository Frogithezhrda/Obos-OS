#include "consoleDriver.h"

static unsigned short position = 0;

void clearScreen()
{
    char* video = (char*)VIDEO_LOCATION;
    char* blank = ' ';
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i += 2) 
    {
        video[i] = blank;
        video[i + 1] = WHITE;
    }
    position = 0;
    updateCursor(position / 2);
}

void printChar(const char character, const int color)
{
    print(&character, color); 
}

void printLine(const char* string, const int color)
{
    print(string, color);  
    print("\n", color); 
}

void printNumber(const int number, const int color)
{

}


void print(const char* string, const int color)
{
    char* video = (char*)VIDEO_LOCATION;
    for (int i = 0; string[i] != '\0'; i++)
    {
        if (string[i] == '\n') 
        {
            int currentChar = position / 2; 
            int currentRow = currentChar / SCREEN_WIDTH;
            int nextRow = currentRow + 1;
            position = nextRow * SCREEN_WIDTH * 2;
            continue;
        }
        
        video[position] = string[i];
        video[position + 1] = color;         
        position += 2;   
    }
    updateCursor(position / 2);
}

void updateCursor(const int cursorPos)
{
    outb(VGA_COMMAND_PORT, 0x0E);
    outb(VGA_DATA_PORT, (cursorPos >> 8) & 0xFF);

    outb(VGA_COMMAND_PORT, 0x0F);
    outb(VGA_DATA_PORT, cursorPos & 0xFF);
}