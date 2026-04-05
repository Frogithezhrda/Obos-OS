#include "gui.h"
#include "apps.h"


unsigned int isGUIInitialized = 0;

static Icon fileIcon;
static Icon consoleIcon;
Button exitButton;

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

//printing characters on screen(using graphics)
static void printCharGUI(char c, unsigned int x, unsigned int y, Color color, unsigned int size)
{

    //is it duplicate code? yes, i dont give a shit
    unsigned char* glyph = font[(unsigned char)c];
    for (unsigned int row = 0; row < 8; row++)
    {
        for (unsigned int col = 0; col < 8; col++)
        {
            if (glyph[row] & (0x80 >> col))
            {
                for (unsigned int sy = 0; sy < size; sy++)
                {
                    for (unsigned int sx = 0; sx < size; sx++)
                    {
                        Pixel p = {x + col * size + sx, y + row * size + sy, color};
                        printPixel(p);
                    }
                }
            }
        }
    }
}

static void printStringGUI(const char* str, unsigned int x, unsigned int y, Color color, unsigned int size)
{
    unsigned int origX = x;
    while (*str)
    {
        if (*str == '\n')
        {
            y += 8 * size;
            x = origX;
        }
        else
        {
            printCharGUI(*str, x, y, color, size);
            x += 8 * size + 1;
        }
        str++;
    }
}



//handling click on window
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

    if (isPointInWindow(&exitButton.label.window, mouseX, mouseY))
    {
        if (exitButton.onClick) exitButton.onClick();
    }

}



//draw functions
void drawButton(Button* button)
{
    if (!button->label.window.isVisible) return;
    drawWindow(&button->label.window);
    unsigned int textLen = 0;
    while (button->label.text[textLen]) textLen++;
    unsigned int textWidth  = textLen * CHAR_W;
    unsigned int textHeight = CHAR_H;
    unsigned int textX = button->label.window.x + (button->label.window.width  - textWidth)  / 2;
    unsigned int textY = button->label.window.y + (button->label.window.height - textHeight) / 2;
    printStringGUI(button->label.text, textX, textY, button->label.textColor, FONT_SCALE);
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
    printStringGUI(label->text, label->window.x + 8, label->window.y - 2 + (label->window.height - CHAR_H) / 2, label->textColor, label->size);
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
    drawLabel(&(Label){ {4, SCREEN_HEIGHT - 60, 100, 40, LIGHT_GREY, VISIBLE}, timeStr, 2,  {82, 204, 255} });
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


//create

Window createWindow(unsigned int x, unsigned int y, unsigned int width, unsigned int height, Color color, char isVisible)
{
    Window win;
    win.bgColor = color;
    win.x = x;
    win.y = y;
    win.width = width;
    win.height = height;
    win.isVisible = isVisible;
    return win;
}

Icon createIcon(Window* window, unsigned char (*iconData)[16], void (*onClick)())
{
    Icon icon;
    icon.window = *window;
    icon.iconData = iconData;
    icon.onClick = onClick;
    return icon;
}

Label createLabel(Window* window, char* text, unsigned int size, Color color)
{
    Label label;
    label.size = size;
    label.window = *window;
    label.textColor = color;
    label.text = text;
    return label;
}

Button createButton(Label* label, void (*onClick)())
{
    Button but;
    but.label = *label;
    but.onClick = onClick;
    return but;
}

//main graphics function
void initalizeWindowGUI()
{
    isGUIInitialized = 1;
    sleep(100);
    Window myWindow = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, LIGHT_BLUE, VISIBLE};
    drawWindow(&myWindow);
    Window toolbar = {0, SCREEN_HEIGHT - 80, SCREEN_WIDTH, 80, LIGHT_GREY, VISIBLE};
    drawWindow(&toolbar);
    Window toolbarDown = {0, SCREEN_HEIGHT - 3, SCREEN_WIDTH, 3, DARK_GREY, VISIBLE};
    Window toolbarUp = {0, SCREEN_HEIGHT - 80, SCREEN_WIDTH, 3, DARK_GREY, VISIBLE};  // thin top border
    Window toolbarLeft = {0, SCREEN_HEIGHT - 80, 3, 80,  DARK_GREY, VISIBLE};  // left border
    Window toolbarRight = {SCREEN_WIDTH - 3, SCREEN_HEIGHT - 80, 3, 80, DARK_GREY, VISIBLE}; // right border
    
    drawWindow(&toolbarDown);
    drawWindow(&toolbarUp);
    drawWindow(&toolbarLeft);
    drawWindow(&toolbarRight);

    initializeApps(&fileIcon, &consoleIcon);

    redrawMouse();
    while (1); 
}

void eraseWindow()
{
    Window myWindow = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, LIGHT_BLUE, VISIBLE};
    drawWindow(&myWindow);
    Window toolbar = {0, SCREEN_HEIGHT - 80, SCREEN_WIDTH, 80, LIGHT_GREY, VISIBLE};
    drawWindow(&toolbar);
    Window toolbarDown = {0, SCREEN_HEIGHT - 3, SCREEN_WIDTH, 3, DARK_GREY, VISIBLE};
    Window toolbarUp = {0, SCREEN_HEIGHT - 80, SCREEN_WIDTH, 3, DARK_GREY, VISIBLE};  // thin top border
    Window toolbarLeft = {0, SCREEN_HEIGHT - 80, 3, 80,  DARK_GREY, VISIBLE};  // left border
    Window toolbarRight = {SCREEN_WIDTH - 3, SCREEN_HEIGHT - 80, 3, 80, DARK_GREY, VISIBLE}; // right border
    
    drawWindow(&toolbarDown);
    drawWindow(&toolbarUp);
    drawWindow(&toolbarLeft);
    drawWindow(&toolbarRight);
    initializeApps(&fileIcon, &consoleIcon);
    redrawMouse();
}