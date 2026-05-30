#include "gui.h"
#include "apps.h"
#include "../Drivers/keyboardDriver.h"
#include "../Apps/filesystem.h"
unsigned int isGUIInitialized = 0;

static Icon fileIcon;
static Icon consoleIcon;
static Icon powerIcon;
static Icon paintIcon;
TextBox* focusedTextBox;

static int isPointInWindow(Window* win, unsigned int x, unsigned int y)
{
    return x >= win->x && x <= win->x + win->width && y >= win->y && y <= win->y + win->height;
}

void drawPixel(Pixel p)
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

void handleMousePress(unsigned int mouseX, unsigned int mouseY)
{
    // check exit buttons first
    for (int i = appCounter - 1; i >= 0; i--)
    {
        if (isPointInWindow(&apps[i].bar.exit.label.window, mouseX, mouseY))
        {
            handleClick(mouseX, mouseY);  // let existing close logic run
            return;
        }
    }

    // check title bars for drag
    for (int i = appCounter - 1; i >= 0; i--)
    {
        if (isPointInWindow(&apps[i].bar.bar, mouseX, mouseY))
        {
            dragState.isDragging = 1;
            dragState.appIndex = i;
            dragState.offsetX = mouseX - apps[i].mainWin.x;
            dragState.offsetY = mouseY - apps[i].mainWin.y;
            return;
        }
    }

    handleClick(mouseX, mouseY);
}

static void eraseApp(App* app)
{
    Window eraseWin = {app->border.x, app->border.y, app->border.width, app->border.height, LIGHT_BLUE, VISIBLE};
    drawWindow(&eraseWin);
}

void handleMouseRelease()
{
    dragState.isDragging = 0;
    dragState.appIndex = -1;
}

void handleMouseMove(unsigned int mouseX, unsigned int mouseY)
{
    if (!dragState.isDragging || dragState.appIndex < 0) return;

    int newX = (int)mouseX - dragState.offsetX;
    int newY = (int)mouseY - dragState.offsetY;

    // clamp to screen
    if (newX < 0) newX = 0;
    if (newY < 20) newY = 20;  // leave room for title bar

    moveApp(&apps[dragState.appIndex], newX, newY);
    eraseWindow();
    redrawMouse();
}

//handling click on window
void handleClick(unsigned int mouseX, unsigned int mouseY)
{
    if (isPointInWindow(&consoleIcon.window, mouseX, mouseY))
    {
        if (consoleIcon.onClick) consoleIcon.onClick();
    }
    else if (isPointInWindow(&fileIcon.window, mouseX, mouseY))
    {
        if (fileIcon.onClick) fileIcon.onClick();
    }
    else if (isPointInWindow(&fileIconFM.window, mouseX, mouseY))
    {
        if (fileIconFM.onClick) fileIconFM.onClick();
    }
    else if (isPointInWindow(&folderIconFM.window, mouseX, mouseY))
    {
        if (folderIconFM.onClick) folderIconFM.onClick();
    }
    else if (isPointInWindow(&deleteIconFM.label.window, mouseX, mouseY))
    {
        if (deleteIconFM.onClick) deleteIconFM.onClick();
    }
    else if (isPointInWindow(&powerIcon.window, mouseX, mouseY))
    {
        if (powerIcon.onClick) powerIcon.onClick();
    }
    else if (isPointInWindow(&paintIcon.window, mouseX, mouseY))
    {
        if (paintIcon.onClick) paintIcon.onClick();
    }
    for (int i = 0; i < fileCount; i++)
    {
        if (isPointInWindow(&fileIcons[i].window, mouseX, mouseY))
        {
            clickedFileIndex = i;
            if (fileIcons[i].onClick) fileIcons[i].onClick();
            break;
        }
    }
    for(int i = 0; i < appCounter; i++)
    {
        App* app = &apps[i];
        if (isPointInWindow(&app->bar.exit.label.window, mouseX, mouseY))
        {
            focusedTextBox = NULL;
            //close app
            for (int j = i; j < appCounter - 1; j++)
            {
                apps[j] = apps[j + 1];
            }
            appCounter--;
            eraseWindow();
            break;
        }
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
    printStringGUI(button->label.text, textX, textY, button->label.textColor, button->label.size);
}

void drawIcon(Icon* icon, unsigned int iconscale)
{
    int scale = iconscale;
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
    unsigned int charH = 8 * label->size;
    printStringGUI(label->text, label->window.x + 4, label->window.y + (label->window.height - charH) / 2, label->textColor, label->size);
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
    drawLabel(&(Label){ {4, SCREEN_HEIGHT - 60, 100, 40, LIGHT_GREY, VISIBLE}, timeStr, 2,  {71, 71, 71} });
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

void drawTextBox(TextBox* tb)
{
    drawLabel(&tb->label);

    // unsigned int charH = 8 * tb->label.size;
    // printStringGUI(tb->label.text, tb->label.window.x + 4, tb->label.window.y + (tb->label.window.height - charH) / 2, tb->label.textColor, tb->label.size);
}


void textBoxHandleKey()
{
    // if (focusedTextBox == 0) return;
    // if(strlen(focusedTextBox->label.text) >= focusedTextBox->maxSize) return;
    keybosGUI(focusedTextBox);
}

void setFocusedTextBox(TextBox* txtBox)
{
    focusedTextBox = txtBox;
}

//create

TextBox createTextBox(Label* label, unsigned int size)
{
    TextBox textBox;
    textBox.label = *label;
    textBox.maxSize = size;
    textBox.cursorPos = 0;
    return textBox;
}

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
    drawTimeLabel(getRTCTime());
    initializeApps(&fileIcon, &consoleIcon, &powerIcon, &paintIcon);

    redrawMouse();
    while (1)
    {
        if(getTicks() % 200 == 0) drawTimeLabel(getRTCTime());
        if (focusedTextBox) textBoxHandleKey();
    }
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
    drawTimeLabel(getRTCTime());
    initializeApps(&fileIcon, &consoleIcon, &powerIcon, &paintIcon);
    redrawMouse();
    for(int i = 0; i < appCounter; i++)
    {
        drawApp(&apps[i]);
    }
}