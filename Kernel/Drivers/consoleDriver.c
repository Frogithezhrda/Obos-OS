#include "consoleDriver.h"

const Color BLACK = {0, 0, 0};
const Color BLUE = {0, 0, 255};
const Color GREEN = {0, 255, 0};
const Color CYAN = {0, 255, 255};
const Color RED = {255, 0, 0};
const Color MAGENTA = {255, 0, 255};
const Color BROWN = {165, 42, 42};
const Color LIGHT_GREY = {211, 211, 211};
const Color DARK_GREY = {169, 169, 169};
const Color LIGHT_BLUE = {173, 216, 230};
const Color LIGHT_GREEN = {144, 238, 144};
const Color LIGHT_CYAN = {224, 255, 255};
const Color LIGHT_RED = {255, 99, 71};
const Color LIGHT_MAGENTA = {255, 0, 255};
const Color YELLOW = {255, 255, 0};
const Color WHITE = {255, 255, 255};


static unsigned int positionX = 0;
static unsigned int positionY = 0;
static unsigned int cursorBlinkingTicks = 0;
static int isVisibleCursor = 0;

static void scrollScreen()
{
    for (unsigned int y = 0; y < SCREEN_HEIGHT - (CHAR_H + 4); y++)
    {
        for (unsigned int x = 0; x < SCREEN_WIDTH; x++)
        {
            Pixel src = {x, y + (CHAR_H + 4), BLACK};
            Color color = getPixelColor(src.x, src.y);
            Pixel p = {x, y, color};
            printPixel(p);
        }
    }

    for (unsigned int y = SCREEN_HEIGHT - (CHAR_H + 4); y < SCREEN_HEIGHT; y++)
    {
        for (unsigned int x = 0; x < SCREEN_WIDTH; x++)
        {
            Pixel p = {x, y, BLACK};
            printPixel(p);
        }
    }
}

static void eraseCursor()
{
    for (unsigned int row = 0; row < CHAR_H + 4; row++)
        for (unsigned int col = 0; col < 2; col++)
        {
            Pixel p = {positionX * CHAR_W + col, positionY * (CHAR_H + 4) + row, BLACK};
            printPixel(p);
        }
}

static void drawCursor(int visible)
{
    Color c = visible ? WHITE : BLACK;
    for (unsigned int row = 0; row < CHAR_H + 4; row++)
    {
        for (unsigned int col = 0; col < 2; col++)
        {
            Pixel p = {positionX * CHAR_W + col, positionY * (CHAR_H + 4) + row, c};
            printPixel(p);
        }
    }
}

void updateCursor()
{
    cursorBlinkingTicks++;
    if (cursorBlinkingTicks >= 30) //blinking every 30 ticks
    {
        isVisibleCursor = !isVisibleCursor;
        drawCursor(isVisibleCursor);
        cursorBlinkingTicks = 0;
    }
}

void clearScreen()
{
    for(unsigned int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for(unsigned int x = 0; x < SCREEN_WIDTH; x++)
        {
            Pixel p = {x, y, BLACK};
            printPixel(p);
        }
    }
    positionX = 0;
    positionY = 0;
}

void printChar(const char character, const Color color)
{
    eraseCursor();
    if(character == '\n')
    {
        positionX = 0;
        positionY++;
        if(positionY >= ROWS)
        {
            scrollScreen();
            positionY = ROWS - 1;
        }
        return;
    }
    else if (character == '\b')
    {
        if (positionX > 0)
        {
            positionX--;
        }
        else if (positionY > 0)
        {
            positionY--;
            positionX = COLS - 1;
        }
        else
        {
            return;
        }

        // clear the entire character cell
        for (unsigned int row = 0; row < CHAR_H; row++)
        {
            for (unsigned int col = 0; col < CHAR_W; col++)
            {
                Pixel p = {positionX * CHAR_W + col, positionY * (CHAR_H + 4) + row, BLACK};
                printPixel(p);
            }
        }
        return;
    }
    printCharVBE(character, positionX * CHAR_W, positionY * (CHAR_H + 4), color);
    positionX++;
}

void printLine(const char* string, const Color color)
{
    print(string, color);
    printChar('\n', color);
}

void printNumber(int number, const Color color)
{
    int isNegative = 0;
    char buffer[12]; //enough for int
    int i = 0;

    if (number < 0)
    {
        isNegative = 1;
        number = -number;
    }

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


void print(const char* string, const Color color)
{
    for(unsigned int i = 0; string[i] != '\0'; i++)
    {
        printChar(string[i], color);
    }
}

void printHexNumber(unsigned int number, const Color color)
{
    char hexChars[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--)
    {
        unsigned char nibble = (number >> (i * 4)) & 0xF;
        printChar(hexChars[nibble], WHITE);
    }
}
