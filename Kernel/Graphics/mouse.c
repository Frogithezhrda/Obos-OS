#include "mouse.h"

int mouseCycle = 0;
char mousePacket[3];
int mouseX = SCREEN_WIDTH / 2;
int mouseY = SCREEN_HEIGHT / 2;
int prevMouseX = SCREEN_WIDTH / 2;
int prevMouseY = SCREEN_HEIGHT / 2;

Color mouseBuffer[8][8];

int shape[8][8] = 
{
    {1,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,0,1,0,0,0,0,0},
    {1,0,0,1,0,0,0,0},
    {1,0,0,0,1,0,0,0},
    {1,1,1,1,1,1,0,0},
    {1,0,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0},
};

static void saveMouseBackground()
{
    for(int y = 0; y < 8; y++)
    {
        for(int x = 0; x < 8; x++)
        {
            if(shape[y][x])
            {
                mouseBuffer[y][x] = getPixelColor(mouseX + x, mouseY + y);
            }
        }
    }
}

static void eraseMouse()
{
    for(int y = 0; y < 8; y++)
    {
        for(int x = 0; x < 8; x++)
        {
            if(shape[y][x])
            {
                Pixel p = {prevMouseX + x, prevMouseY + y, mouseBuffer[y][x]};
                printPixel(p);
            }
        }
    }
}

static void drawMouse()
{
    for(int y = 0; y < 8; y++)
    {
        for(int x = 0; x < 8; x++)
        {
            if(shape[y][x])
            {
                Pixel p = {mouseX + x, mouseY + y, WHITE};
                printPixel(p);
            }
        }
    }
}

static void processMousePacket()
{
    eraseMouse();
    int xMove = mousePacket[1];
    int yMove = mousePacket[2];

    mouseX += xMove;
    mouseY -= yMove;

    //clamp to screen
    if (mouseX < 0) mouseX = 0;
    if (mouseY < 0) mouseY = 0;
    if (mouseX >= SCREEN_WIDTH) mouseX = SCREEN_WIDTH - 1;
    if (mouseY >= SCREEN_HEIGHT) mouseY = SCREEN_HEIGHT - 1;

    saveMouseBackground();

    drawMouse();
    prevMouseX = mouseX;
    prevMouseY = mouseY;
}

static void mouseWaitRead()
{
    for (int i = 0; i < 100000; i++)
    {
        if (inb(0x64) & 1)
            return;
    }
}

static void mouseWaitWrite()
{
    for (int i = 0; i < 100000; i++)
    {
        if (!(inb(0x64) & 2))
            return;
    }
}

static void mouseWrite(unsigned char data)
{
    mouseWaitWrite();
    outb(0x64, 0xD4);
    mouseWaitWrite();
    outb(0x60, data);
}

static unsigned char mouseRead()
{
    mouseWaitRead();
    return inb(0x60);
}

void mouseIRQHandler()
{
    unsigned char data = inb(0x60); // MUST READ

    switch(mouseCycle)
    {
        case 0:
            if (!(data & 0x08)) break; //bit 3 must be 1
            mousePacket[0] = data;
            mouseCycle++;
            break;

        case 1:
            mousePacket[1] = data;
            mouseCycle++;
            break;

        case 2:
            mousePacket[2] = data;
            mouseCycle = 0;

            processMousePacket();
            break;
    }
    endOfInterrupt(12); //mouse is on IRQ 12
}

void mouseInit()
{
    mouseWaitWrite();
    outb(0x64, 0xA8);
    printLineW("Enabling mouse...");

    mouseWaitWrite();
    outb(0x64, 0x20);
    mouseWaitRead();
    unsigned char status = inb(0x60);

    status |= 0x02;
    printLineW("Enabling mouse IRQ...");

    mouseWaitWrite();
    outb(0x64, 0x60);
    mouseWaitWrite();
    outb(0x60, status);

    mouseWrite(0xF6);
    mouseRead();
    mouseWrite(0xF4);
    mouseRead();
}