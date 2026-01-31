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
    char buffer[2];
    buffer[0] = character;
    buffer[1] = '\0';
    print(buffer, color);
}

void printLine(const char* string, const int color)
{
    print(string, color);  
    print("\n", color); 
}

void printNumber(int number, const int color)
{
    int isNegative = 0;
    char buffer[12]; //enough for int
    int i = 0;

    if (number == 0) 
    {
        printChar('0', color);
        return;
    }

    while (number > 0)
    {
        buffer[i] = (number % 10) + '0';
        i++;
        number /= 10;
    }
    if (isNegative) 
    {
        buffer[i] = '-';
        i++;
    }
    //printing in reverse for that
    for (int j = i - 1; j >= 0; j--) 
    {
        printChar(buffer[j], color);
    }
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

void printHexNumber(unsigned int number, const int color)
{
    char hexChars[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--)
    {
        unsigned char nibble = (number >> (i * 4)) & 0xF;
        printChar(hexChars[nibble], WHITE);
    }
    printLine("", WHITE);
}

void updateCursor(const int cursorPos)
{
    outb(VGA_COMMAND_PORT, 0x0E);
    outb(VGA_DATA_PORT, (cursorPos >> 8) & 0xFF);

    outb(VGA_COMMAND_PORT, 0x0F);
    outb(VGA_DATA_PORT, cursorPos & 0xFF);
}