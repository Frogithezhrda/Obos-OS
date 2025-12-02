#include "consoleDriver.h"

void clearScreen()
{
    char* video = (char*)VIDEO_LOCATION;
    char* blank = ' ';
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i += 2) 
    {
        video[i] = blank;
        video[i + 1] = 0x0F;
    }

}

void print(const char* string)
{
    char* video = (char*)VIDEO_LOCATION;
    
    for (int i = 0; string[i] != '\0'; i++)
    {
        video[i * 2] = string[i];
        video[i * 2 + 1] = 0x0F;  
    }
}