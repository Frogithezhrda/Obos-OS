#include "gui.h"

char icon[8][8] = 
{
    {0,0,1,1,1,1,0,0},
    {0,1,0,0,0,0,1,0},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {0,1,0,0,0,0,1,0},
    {0,0,1,1,1,1,0,0},
};


void initalizeWindowGUI()
{
    Window myWindow = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, LIGHT_BLUE, VISIBLE};
    drawWindow(&myWindow);
    Window toolbar = {0, SCREEN_HEIGHT - 40, SCREEN_WIDTH, 40, LIGHT_GREY, VISIBLE};
    drawWindow(&toolbar);
    Icon toolbarIcon = {0, SCREEN_HEIGHT - 40, 40, 40, {69, 174, 255}, VISIBLE};
    toolbarIcon.iconData = (char*)icon;
    drawIcon(&toolbarIcon);
    while (1); 
}

void drawButton(Button* button) 
{
    drawWindow(&button->window);

}

void drawIcon(Icon* icon)
{
    unsigned int scale = icon->window.width / 8; //assuming the icon data is 8x8
    drawWindow(&icon->window);
    for(int i = 0; i < 8; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            if(icon->iconData[i * 8 + j]) //if the pixel is not transparent
            {
                for(int y = 0; y < scale; y++)
                {
                    for(int x = 0; x < scale; x++)
                    {
                        Pixel p = { icon->window.x + j * scale + x, icon->window.y + i * scale + y, {60, 98, 128} };
                        printPixel(p);
                    }
                }
            }
        } 
    }
}

void drawLabel(Label* label)
{
    drawWindow(&label->window);
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