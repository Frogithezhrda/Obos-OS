#include "apps.h"
#include "../Apps/terminal.h"
#include "../Apps/filesystem.h"
#include "../Apps/paint.h"


int appCounter = 0;
App apps[10]; //max 10 apps at once we can increase this later if needed


static void exitApp()
{
    focusedTextBox = NULL;
    for (int i = 0; i < appCounter; i++)
    {
        if (apps[i].bar.exit.onClick == exitApp) 
        {
            for (int j = i; j < appCounter - 1; j++)
                apps[j] = apps[j + 1];
            appCounter--;
            break;
        }
    }
    eraseWindow();
    redrawMouse();
    for (int i = 0; i < appCounter; i++)
        drawApp(&apps[i]);
        // apps[i].isDirty = 1;

}

static void powerOff()
{
    outw(0x604, 0x2000);
}

InfoBar appBar(unsigned int x, unsigned int y, unsigned int width, char* title)
{
    InfoBar bar;
    //current bar implementaion later with a struct
    Window barWin = createWindow(x, y - 5, width, 20, LIGHT_GREY, VISIBLE);
    Window titleWin = createWindow(x + 2, y - 5, 20, 20, LIGHT_GREY, VISIBLE);
    Label titleLabel = createLabel(&titleWin, title, 1, BLACK);
    Window buttonWin = createWindow(x + width - 20, y - 5, 20, 20, LIGHT_GREY, VISIBLE);
    Label buttonLabel = createLabel(&buttonWin, "X", 1, RED);
    bar.exit = createButton(&buttonLabel, exitApp);
    bar.exit.onClick = exitApp;
    bar.bar = barWin;
    bar.title = titleLabel;
    // drawWindow(&bar);
    // drawLabel(&titleLabel);
    // drawButton(&exitButton);
    return bar;
}

void drawApp(App* app)
{
    drawWindow(&app->border);
    drawWindow(&app->mainWin);

    drawWindow(&app->bar.bar);
    drawLabel(&app->bar.title);
    drawButton(&app->bar.exit);
}


static void openTerminal()
{
    App termApp;
    Window termWin = {100, 100, 600, 400, BLACK, VISIBLE};
    termApp.mainWin = termWin;
    // drawWindow(&termWin);
    //draw a border
    Window border = {termWin.x - 2, termWin.y - 2, termWin.width + 4, termWin.height + 4, DARK_GREY, VISIBLE};
    // drawWindow(&border);
    termApp.border = border;
    termApp.bar = appBar(termWin.x, termWin.y, termWin.width, "Terminal");
    termApp.id = appCounter;
    termApp.isDirty = 0;
    apps[appCounter++] = termApp;

    drawApp(&termApp);
    // terminalOpen(&termApp);
}

void openFileManager()
{
    App fileApp;
    Window termWin = {400, 200, 600, 400, {239, 255, 172}, VISIBLE};
    fileApp.mainWin = termWin;
    //draw a border
    Window border = {termWin.x - 2, termWin.y - 2, termWin.width + 4, termWin.height + 4, DARK_GREY, VISIBLE};
    fileApp.border = border;
    fileApp.bar = appBar(termWin.x, termWin.y, termWin.width, "File System");
        fileApp.id = appCounter;
    fileApp.isDirty = 0;
    apps[appCounter++] = fileApp;

    drawApp(&fileApp);
    drawCreationIcon(&fileApp);
    drawFsFiles();
    // Label title = {{110, 100, 200, 20, LIGHT_GREY, VISIBLE}, "File Manager", BLACK};
    // drawLabel(&title);
}

void initializeApps(Icon* fileIcon, Icon* consoleIcon, Icon* powerIcon, Icon* paintIcon)
{
    paintIcon->window = (Window){SCREEN_WIDTH - 320, SCREEN_HEIGHT - 80, 80, 80, {69, 174, 255}, VISIBLE};
    paintIcon->iconData = paint16;
    paintIcon->onClick = openPaint;
    drawIcon(paintIcon, 5);
    
    // //filesystem icon
    fileIcon->window = (Window){SCREEN_WIDTH - 240, SCREEN_HEIGHT - 80, 80, 80, {69, 174, 255}, VISIBLE};
    fileIcon->iconData = folder16;
    fileIcon->onClick = openFileManager;
    drawIcon(fileIcon, 5);
    // //console
    consoleIcon->window = (Window){SCREEN_WIDTH - 160, SCREEN_HEIGHT - 80, 80, 80, {69, 174, 255}, VISIBLE};
    consoleIcon->iconData = console16;
    consoleIcon->onClick = openTerminal;
    drawIcon(consoleIcon, 5);

    powerIcon->window = (Window){SCREEN_WIDTH - 80, SCREEN_HEIGHT - 80, 80, 80, {69, 174, 255}, VISIBLE};
    powerIcon->iconData = power16;
    powerIcon->onClick = powerOff;
    drawIcon(powerIcon, 5);
}