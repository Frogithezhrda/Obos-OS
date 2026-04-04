#include "apps.h"


static void exitApp()
{
    eraseWindow();
    redrawMouse();
}

static void appBar(unsigned int x, unsigned int y, unsigned int width, char* title)
{
    //current bar implementaion later with a struct
    Window bar = {x, y - 5, width, 20, LIGHT_GREY, VISIBLE};
    Label titleLabel = {{x + 2, y - 5, 20, 20, LIGHT_GREY, VISIBLE}, title, 1, BLACK};
    //{{x + width - 20, y - 5, 20, 20, LIGHT_GREY, VISIBLE}, "X", RED, exitApp};
    exitButton.window.x = x + width - 20;
    exitButton.window.y = y - 5;
    exitButton.window.width = 20;
    exitButton.window.height = 20;
    exitButton.window.bgColor = LIGHT_GREY;
    exitButton.window.isVisible = VISIBLE;
    exitButton.text = "X";
    exitButton.textColor = RED;
    exitButton.onClick = exitApp;
    drawWindow(&bar);
    drawLabel(&titleLabel);
    drawButton(&exitButton);
}


static void openTerminal()
{
    Window termWin = {100, 100, 600, 400, BLACK, VISIBLE};
    drawWindow(&termWin);
    //draw a border
    Window border = {98, 98, 604, 404, DARK_GREY, VISIBLE};
    drawWindow(&border);
    drawWindow(&termWin);
    //title bar
    // Window titleBar = {98, 98, 604, 24, LIGHT_GREY};
    // drawWindow(&titleBar);
    // Label title = {{110, 100, 200, 20, LIGHT_GREY, VISIBLE}, "Terminal", BLACK};
    // drawLabel(&title);
    appBar(100, 100, 600, "Terminal");

}

static void openFileManager()
{
    Window termWin = {100, 120, 600, 400, {239, 255, 172}, VISIBLE};
    drawWindow(&termWin);
    //draw a border
    Window border = {98, 98, 604, 404, DARK_GREY, VISIBLE};
    drawWindow(&border);
    drawWindow(&termWin);
    appBar(100, 100, 600, "File System");
    // Label title = {{110, 100, 200, 20, LIGHT_GREY, VISIBLE}, "File Manager", BLACK};
    // drawLabel(&title);
}


void initializeApps(Icon* fileIcon, Icon* consoleIcon)
{
    //filesystem icon
    fileIcon->window = (Window){SCREEN_WIDTH - 160, SCREEN_HEIGHT - 80, 40, 40, {69, 174, 255}, VISIBLE};
    fileIcon->iconData = folder16;
    fileIcon->onClick = openFileManager;
    drawIcon(fileIcon);

    // //console
    consoleIcon->window = (Window){SCREEN_WIDTH - 80, SCREEN_HEIGHT - 80, 40, 40, {69, 174, 255}, VISIBLE};
    consoleIcon->iconData = console16;
    consoleIcon->onClick = openTerminal;
    drawIcon(consoleIcon);
}