#include "gui.h"

unsigned int isGUIInitialized = 0;

static Icon fileIcon;
static Icon consoleIcon;

static int isPointInWindow(Window* win, unsigned int x, unsigned int y)
{
    return x >= win->x && x <= win->x + win->width * 2 && y >= win->y && y <= win->y + win->height * 2;
}

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

void handleClick(unsigned int mouseX, unsigned int mouseY)
{
    if (isPointInWindow(&consoleIcon.window, mouseX, mouseY))
    {
        if (consoleIcon.onClick) consoleIcon.onClick();
    }

    if (isPointInWindow(&fileIcon.window, mouseX, mouseY))
    {
        if (fileIcon.onClick) fileIcon.onClick();
    }
}

void openTerminal()
{
    Window termWin = {100, 100, 600, 400, BLACK};
    drawWindow(&termWin);
    //draw a border
    Window border = {98, 98, 604, 404, DARK_GREY};
    drawWindow(&border);
    drawWindow(&termWin);
    //title bar
    Window titleBar = {98, 98, 604, 24, LIGHT_GREY};
    drawWindow(&titleBar);
    Label title = {{110, 100, 200, 20, LIGHT_GREY, VISIBLE}, "Terminal", BLACK};
    drawLabel(&title);
}

void openFileManager()
{
    Window fmWin = {100, 100, 600, 400, WHITE};
    drawWindow(&fmWin);
    Window border = {98, 98, 604, 404, DARK_GREY};
    drawWindow(&border);
    drawWindow(&fmWin);
    Window titleBar = {98, 98, 604, 24, LIGHT_GREY};
    drawWindow(&titleBar);
    Label title = {{110, 100, 200, 20, LIGHT_GREY, VISIBLE}, "File Manager", BLACK};
    drawLabel(&title);
}

static void printCharGUI(char c, unsigned int x, unsigned int y, Color color)
{

    //is it duplicate code? yes, i dont give a shit
    unsigned char* glyph = font[(unsigned char)c];
    for (unsigned int row = 0; row < 8; row++)
    {
        for (unsigned int col = 0; col < 8; col++)
        {
            if (glyph[row] & (0x80 >> col))
            {
                for (unsigned int sy = 0; sy < GUI_FONT_SCALE; sy++)
                {
                    for (unsigned int sx = 0; sx < GUI_FONT_SCALE; sx++)
                    {
                        Pixel p = {x + col * GUI_FONT_SCALE + sx, y + row * GUI_FONT_SCALE + sy, color};
                        printPixel(p);
                    }
                }
            }
        }
    }
}

static void printStringGUI(const char* str, unsigned int x, unsigned int y, Color color)
{
    unsigned int origX = x;
    while (*str)
    {
        if (*str == '\n')
        {
            y += 8 * GUI_FONT_SCALE;
            x = origX;
        }
        else
        {
            printCharGUI(*str, x, y, color);
            x += 8 * GUI_FONT_SCALE + 1;
        }
        str++;
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
    Window toolbarDown = {0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, 10, DARK_GREY, VISIBLE};
    drawWindow(&toolbarDown);
    Window toolbarUp = {0, SCREEN_HEIGHT - 80, SCREEN_WIDTH, 10, DARK_GREY, VISIBLE};
    drawWindow(&toolbarUp);

    //filesystem icon
    fileIcon.window = (Window){SCREEN_WIDTH - 160, SCREEN_HEIGHT - 80, 40, 40, {69, 174, 255}, VISIBLE};
    fileIcon.iconData = folder16;
    fileIcon.onClick = openFileManager;
    drawIcon(&fileIcon);

    // //console
    consoleIcon.window = (Window){SCREEN_WIDTH - 80, SCREEN_HEIGHT - 80, 40, 40, {69, 174, 255}, VISIBLE};
    consoleIcon.iconData = console16;
    consoleIcon.onClick = openTerminal;
    drawIcon(&consoleIcon);
    
    while (1); 
}

void drawButton(Button* button)
{
    if (!button->window.isVisible) return;
    drawWindow(&button->window);

    unsigned int textLen = 0;
    while (button->text[textLen]) textLen++;
    unsigned int textWidth  = textLen * CHAR_W;
    unsigned int textHeight = CHAR_H;
    unsigned int textX = button->window.x + (button->window.width  - textWidth)  / 2;
    unsigned int textY = button->window.y + (button->window.height - textHeight) / 2;
    printStringGUI(button->text, textX, textY, button->textColor);
}
void drawIcon(Icon* icon)
{
    int scale = 5;
    if (!icon->window.isVisible) return;
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            int pixel = icon->iconData[i][j];

            if (pixel == 0) continue; //transparent

            Color color = palette[pixel];

            drawPixel((Pixel){ icon->window.x + j * scale, icon->window.y + i * scale, color });
        }
    }
}

void drawLabel(Label* label)
{
    if (!label->window.isVisible) return;
    drawWindow(&label->window);
    printStringGUI(label->text, label->window.x + 8, label->window.y - 2 + (label->window.height - CHAR_H) / 2, label->textColor);
}

void drawTimeLabel(Time time)
{
    char timeStr[6];
    timeStr[0] = (time.hours / 10) + '0';
    timeStr[1] = (time.hours % 10) + '0';
    timeStr[2] = ':';
    timeStr[3] = (time.minutes / 10) + '0';
    timeStr[4] = (time.minutes % 10) + '0';
    timeStr[5] = '\0';
    drawLabel(&(Label){ {0, SCREEN_HEIGHT - 70, 100, 60, {222, 222, 222}, VISIBLE}, timeStr, {102, 102, 102} });
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