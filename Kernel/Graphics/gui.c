#include "gui.h"

void initalizeWindowGUI()
{
    Window myWindow = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, LIGHT_BLUE, VISIBLE};
    drawWindow(&myWindow);
    Window toolbar = {0, SCREEN_HEIGHT - 40, SCREEN_WIDTH, 40, LIGHT_GREY, VISIBLE};
    drawWindow(&toolbar);
    while (1); 
}


void drawWindow(Window* win) 
{
    if(!win->isVisible) return;
    for(int row = 0; row < win->height; row++) 
    {
        for(int col = 0; col < win->width; col++) 
        {
            Pixel p = {win->x + col, win->y + row, win->bgColor};
            printPixel(p);
        }
    }
}