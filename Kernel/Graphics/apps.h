#ifndef APPS_H
#define APPS_H

#include "gui.h"
#include "mouse.h"

typedef struct InfoBar
{
    Window bar;
    Label title;
    Button* exit;
} InfoBar;


typedef struct App
{
    InfoBar bar;
    Window border;
    Window mainWin;
} App;



void initializeApps(Icon* fileIcon, Icon* consoleIcon, Icon* powerIcon, Icon* paintIcon);
void openFileManager();
InfoBar appBar(unsigned int x, unsigned int y, unsigned int width, char* title);
void drawApp(App* app);

#endif