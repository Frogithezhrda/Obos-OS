#include "taskbar.h"
#include "../Graphics/mouse.h"
#include "wm.h"

#define START_W 60
#define BTN_W 120
#define BTN_H (TASKBAR_H_PX - 8)
#define BTN_GAP 4
#define BTN_X0 (4 + START_W + 8)

static int tbPrevLeft = 0;


static void drawClock(void)
{
    static unsigned int lastRead = 0;
    static int h = 0, m = 0;
    unsigned int now = getTicks();
    if (lastRead == 0 || now - lastRead >= TIMER_HZ) 
    {
        Time t = getRTCTime();
        h = t.hours;
        m = t.minutes;
        lastRead = now ? now : 1; 
    }

    char text[6] = { '0' + h / 10, '0' + h % 10, ':', '0' + m / 10, '0' + m % 10, 0 };
    int x = SCREEN_WIDTH - 5 * 8 - 10;
    gfxFillRect(x - 6, TASKBAR_Y + 4, 5 * 8 + 12, BTN_H, GRAY);
    gfxFillRect(x - 6, TASKBAR_Y + 4, 5 * 8 + 12, 1, DARK);
    gfxFillRect(x - 6, TASKBAR_Y + 4, 1, BTN_H, DARK);
    gfxDrawString(text, x, TASKBAR_Y + 4 + (BTN_H - 8) / 2, BLACK);
}

// ---------- buttons ----------
static void drawButton(int x, int y, int w, int h, const char* label, int pressed)
{
    gfxFillRect(x, y, w, h, GRAY);
    Color tl = pressed ? DARK : WHITE, br = pressed ? WHITE : DARK;
    gfxFillRect(x, y, w, 1, tl);
    gfxFillRect(x, y, 1, h, tl);
    gfxFillRect(x, y + h - 1, w, 1, br);
    gfxFillRect(x + w - 1, y, 1, h, br);

    int maxChars = (w - 8) / 8;
    char buf[32]; int n = 0;
    while (label[n] && n < maxChars && n < 31) { buf[n] = label[n]; n++; }
    buf[n] = 0;
    gfxDrawString(buf, x + 4 + pressed, y + (h - 8) / 2 + pressed, BLACK);
}

static int inRect(int px, int py, int x, int y, int w, int h)
{
    return px >= x && px < x + w && py >= y && py < y + h;
}

void taskbarUpdate(void)
{
    if (mouseLeft && !tbPrevLeft && mouseY >= TASKBAR_Y)
    {
        int slot = 0;
        for (int id = 0; id < wmNextId(); id++)
        {
            int idx = wmIndexOfId(id);
            if (idx < 0) continue;
            int bx = BTN_X0 + slot * (BTN_W + BTN_GAP);
            if (inRect(mouseX, mouseY, bx, TASKBAR_Y + 4, BTN_W, BTN_H))
            {
                wmFocus(idx);
                break;
            }
            slot++;
        }
        // TODO: Start button click at inRect(mouseX, mouseY, 4, TASKBAR_Y + 4, START_W, BTN_H)
    }
    tbPrevLeft = mouseLeft;
}

void taskbarDraw(void)
{
    gfxFillRect(0, TASKBAR_Y, SCREEN_WIDTH, TASKBAR_H_PX, GRAY);
    gfxFillRect(0, TASKBAR_Y, SCREEN_WIDTH, 1, WHITE);

    drawButton(4, TASKBAR_Y + 4, START_W, BTN_H, "Start", 0);

    int slot = 0;
    for (int id = 0; id < wmNextId(); id++)
    {
        int idx = wmIndexOfId(id);
        if (idx < 0) continue;
        drawButton(BTN_X0 + slot * (BTN_W + BTN_GAP), TASKBAR_Y + 4, BTN_W, BTN_H,
                   wmTitle(idx), wmIsFocused(idx));
        slot++;
    }
    drawClock();
}