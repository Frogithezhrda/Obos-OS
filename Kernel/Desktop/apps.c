#include "apps.h"
#include "wm.h"
#include "../Graphics/gfx.h"
#include "../Drivers/timerDriver.h"
#include "filemgr.h"
#include "editor.h"

typedef struct App
{
    const char* name;
    unsigned char (*icon)[16];
    int w, h;
    const char* title;
    Color bg;
    DrawFn draw;
    int winId;
    ClickFn click;
    KeyFn key;
    Color titleColor;
} App;

#define APP(...) {.winId = -1, __VA_ARGS__}

static unsigned char iconGeneric[16][16] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,2,2,2,2,2,2,2,2,2,2,2,2,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,3,3,3,3,3,3,3,3,3,3,3,3,1,0},
    {0,1,3,3,3,3,3,3,3,3,3,3,3,3,1,0},
    {0,1,3,3,3,3,3,3,3,3,3,3,3,3,1,0},
    {0,1,3,3,3,3,3,3,3,3,3,3,3,3,1,0},
    {0,1,3,3,3,3,3,3,3,3,3,3,3,3,1,0},
    {0,1,3,3,3,3,3,3,3,3,3,3,3,3,1,0},
    {0,1,3,3,3,3,3,3,3,3,3,3,3,3,1,0},
    {0,1,3,3,3,3,3,3,3,3,3,3,3,3,1,0},
    {0,1,3,3,3,3,3,3,3,3,3,3,3,3,1,0},
    {0,1,3,3,3,3,3,3,3,3,3,3,3,3,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};


// ---- app window contents (called every frame) ----
static void drawAbout(int x, int y, int w, int h)
{
    gfxDrawString("OBOS desktop", x + 8, y + 8, BLACK);
    gfxDrawString("Hello from your own OS!", x + 8, y + 24, BLACK);
}

static void drawSystem(int x, int y, int w, int h)
{
    char buf[16];
    utoa10(getTicks() / TIMER_HZ, buf);
    gfxDrawString("Uptime (s):", x + 8, y + 8, BLACK);
    gfxDrawString(buf, x + 8 + 12 * 8, y + 8, BLACK);
    gfxDrawString("Screen: 1024x768x24", x + 8, y + 24, BLACK);
}

static App apps[] =
{
    APP(.name = "About", .w = 280, .h = 100, .title = "About OBOS", .bg = {230, 230, 230}, .draw = drawAbout),
    APP(.name = "System", .w = 260, .h = 90, .draw = drawSystem),
    APP(.name = "Files", .w = 360, .h = 260, .title = "File Manager", .draw = filemgrDraw, .click = filemgrClick, .key = filemgrKey, .titleColor = {255, 212, 0}),
    APP(.name = "Editor", .w = 420, .h = 300, .title = "Text Editor", .draw = editorDraw, .click = editorClick, .key = editorKey),
};


int appCount(void) { return sizeof(apps) / sizeof(apps[0]); }
const char* appName(int i) { return apps[i].name; }
unsigned char (*appIcon(int i))[16] { return apps[i].icon ? apps[i].icon : iconGeneric; }

void appLaunch(int i)
{
    if (i < 0 || i >= appCount()) return;

    if (apps[i].winId >= 0)
    {
        int idx = wmIndexOfId(apps[i].winId);
        if (idx >= 0) { wmFocus(idx); return; }
    }

    static int cascade = 0;
    int x = 140 + cascade * 28, y = 40 + cascade * 28;
    cascade = (cascade + 1) % 8;

    int w = apps[i].w ? apps[i].w : 300;
    int h = apps[i].h ? apps[i].h : 200;
    const char* title = apps[i].title ? apps[i].title : apps[i].name;
    Color bg = apps[i].bg;
    if (!bg.r && !bg.g && !bg.b) bg = (Color){255, 255, 255};
    apps[i].winId = wmAddWindow(x, y, w, h, title, bg, apps[i].draw);

    if (apps[i].winId >= 0 && apps[i].click)
        wmSetClick(apps[i].winId, apps[i].click);

    if (apps[i].winId >= 0 && apps[i].key) wmSetKey(apps[i].winId, apps[i].key);\
    if (apps[i].winId >= 0) wmSetTitleColor(apps[i].winId, apps[i].titleColor);
}

void appLaunchByName(const char* name)
{
    for (int i = 0; i < appCount(); i++)
    {
        if (strcmp(apps[i].name, name) == 0)
        {
            appLaunch(i);
            return;
        }
    }
}