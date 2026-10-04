#include "desktop.h"
#include "apps.h"
#include "wm.h"
#include "taskbar.h"
#include "../Graphics/mouse.h"
#include "../Drivers/timerDriver.h"

#define ICON_SCALE 2
#define ICON_PX (16 * ICON_SCALE)
#define CELL_W 80
#define CELL_H 72
#define MARGIN 16
#define DOUBLE_CLICK_TICKS (TIMER_HZ / 2) // 500 ms

static int selected = -1;
static int lastClickIcon = -1;
static unsigned int lastClickTick = 0;
static int prevLeft = 0;

static void iconPos(int i, int* x, int* y)
{
    int perCol = (TASKBAR_Y - MARGIN) / CELL_H;
    if (perCol < 1) perCol = 1;
    *x = MARGIN + (i / perCol) * CELL_W;
    *y = MARGIN + (i % perCol) * CELL_H;
}

static int iconAt(int px, int py)
{
    for (int i = 0; i < appCount(); i++)
    {
        int x, y; iconPos(i, &x, &y);
        if (px >= x && px < x + CELL_W && py >= y && py < y + CELL_H) return i;
    }
    return -1;
}

void desktopUpdate(void)
{
    if (mouseLeft && !prevLeft && mouseY < TASKBAR_Y && !wmConsumedClick())
    {
        int i = iconAt(mouseX, mouseY);
        unsigned int now = getTicks();

        if (i >= 0 && i == lastClickIcon && now - lastClickTick <= DOUBLE_CLICK_TICKS)
        {
            appLaunch(i);
            lastClickIcon = -1;
        }
        else
        {
            lastClickIcon = i;
            lastClickTick = now;
        }
        selected = i;
    }
    prevLeft = mouseLeft;
}

void desktopDraw(void)
{
    Color white = {255, 255, 255}, hl = {0, 0, 128};

    for (int i = 0; i < appCount(); i++)
    {
        int x, y; iconPos(i, &x, &y);
        gfxDrawIcon(x + (CELL_W - ICON_PX) / 2, y + 4, appIcon(i), ICON_SCALE);

        const char* name = appName(i);
        int len = 0; while (name[len]) len++;
        int lx = x + (CELL_W - len * 8) / 2;
        int ly = y + 4 + ICON_PX + 6;

        if (i == selected) gfxFillRect(lx - 2, ly - 2, len * 8 + 4, 12, hl);
        gfxDrawString(name, lx, ly, white);
    }
}