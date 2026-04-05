#include "apps.h"
#include "../Apps/terminal.h"

static void exitApp()
{
    eraseWindow();
    redrawMouse();
}

static InfoBar appBar(unsigned int x, unsigned int y, unsigned int width, char* title)
{
    InfoBar bar;
    //current bar implementaion later with a struct
    Window barWin = createWindow(x, y - 5, width, 20, LIGHT_GREY, VISIBLE);
    Window titleWin = createWindow(x + 2, y - 5, 20, 20, LIGHT_GREY, VISIBLE);
    Label titleLabel = createLabel(&titleWin, title, 1, BLACK);
    Window buttonWin = createWindow(x + width - 20, y - 5, 20, 20, LIGHT_GREY, VISIBLE);
    Label buttonLabel = createLabel(&buttonWin, "X", 1, RED);
    exitButton = createButton(&buttonLabel, exitApp);
    bar.bar = barWin;
    bar.title = titleLabel;
    bar.exit = &exitButton;
    // drawWindow(&bar);
    // drawLabel(&titleLabel);
    // drawButton(&exitButton);
    return bar;
}

static void drawApp(App* app)
{
    drawWindow(&app->border);
    drawWindow(&app->mainWin);

    drawWindow(&app->bar.bar);
    drawLabel(&app->bar.title);
    drawButton(app->bar.exit);
}


static void openTerminal()
{
    App termApp;
    Window termWin = {100, 100, 600, 400, BLACK, VISIBLE};
    termApp.mainWin = termWin;
    // drawWindow(&termWin);
    //draw a border
    Window border = {98, 98, 604, 404, DARK_GREY, VISIBLE};
    // drawWindow(&border);
    termApp.border = border;
    termApp.bar = appBar(100, 100, 600, "Terminal");

    drawApp(&termApp);
    terminalOpen(&termApp);
}

static void openFileManager()
{
    App fileApp;
    Window termWin = {100, 100, 600, 400, {239, 255, 172}, VISIBLE};
    fileApp.mainWin = termWin;
    //draw a border
    Window border = {98, 98, 604, 404, DARK_GREY, VISIBLE};
    fileApp.border = border;
    fileApp.bar = appBar(100, 100, 600, "File System");

    drawApp(&fileApp);
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