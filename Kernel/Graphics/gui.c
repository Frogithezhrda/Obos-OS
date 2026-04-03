#include "gui.h"

unsigned int isGUIInitialized = 0;

static void drawPixel(Pixel p)
{
    int baseX = p.x;
    int baseY = p.y;

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            Pixel newP = {baseX + j, baseY + i, p.color };
            printPixel(newP);
        }
    }
}

void initalizeWindowGUI()
{
    isGUIInitialized = 1;
    sleep(400);
    Window myWindow = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, LIGHT_BLUE, VISIBLE};
    drawWindow(&myWindow);
    Window toolbar = {0, SCREEN_HEIGHT - 80, SCREEN_WIDTH, 80, LIGHT_GREY, VISIBLE};
    drawWindow(&toolbar);
    Icon toolbarIcon = {0, SCREEN_HEIGHT - 80, 40, 40, {69, 174, 255}, VISIBLE};
    // toolbarIcon.iconData = (char*)icon;
    drawIcon(&toolbarIcon);
    while (1); 
}

void drawButton(Button* button) 
{
    drawWindow(&button->window);

}

void drawIcon(Icon* icon)
{
    int scale = 5;

    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            int pixel = folder16[i][j];

            if (pixel == 0) continue; //transparent

            Color color = palette[pixel];

            drawPixel((Pixel){ icon->window.x + j * scale, icon->window.y + i * scale, color });
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