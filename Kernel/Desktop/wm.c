#include "wm.h"
#include "taskbar.h"
#define MAX_WINDOWS 16
#define TITLE_H 16

typedef struct { int x, y, w, h; const char* title; Color bg; int id; } Window;

static int nextId = 0;
static Window wins[MAX_WINDOWS];
static int winCount = 0;
static int dragging = -1, dragDX, dragDY, prevLeft = 0;

int wmNextId(void) { return nextId; }

int wmIndexOfId(int id)
{
    for (int i = 0; i < winCount; i++) if (wins[i].id == id) return i;
    return -1;
}


void wmAddWindow(int x, int y, int w, int h, const char* title, Color bg)
{
    if (winCount < MAX_WINDOWS) wins[winCount++] = (Window){x, y, w, h, title, bg, nextId++};
}

static void wmRaise(int i)
{
    Window w = wins[i];
    for (int k = i; k < winCount - 1; k++) wins[k] = wins[k + 1];
    wins[winCount - 1] = w;
}

static int wmHit(int px, int py)
{
    for (int i = winCount - 1; i >= 0; i--)
        if (px >= wins[i].x && px < wins[i].x + wins[i].w &&
            py >= wins[i].y && py < wins[i].y + wins[i].h + TITLE_H)
            return i;
    return -1;
}

void wmUpdate()
{
    if (dragging < 0 && mouseY >= TASKBAR_Y) { prevLeft = mouseLeft; return; }
    if (mouseLeft && !prevLeft)
    {
        int i = wmHit(mouseX, mouseY);
        if (i >= 0)
        {
            wmRaise(i);
            i = winCount - 1;
            if (mouseY < wins[i].y + TITLE_H)
            {
                dragging = i;
                dragDX = mouseX - wins[i].x;
                dragDY = mouseY - wins[i].y;
            }
        }
    }
    if (!mouseLeft) dragging = -1;
    if (dragging >= 0)
    {
        wins[dragging].x = mouseX - dragDX;
        wins[dragging].y = mouseY - dragDY;
        if (wins[dragging].y > TASKBAR_Y - TITLE_H) wins[dragging].y = TASKBAR_Y - TITLE_H;
        if (wins[dragging].y < 0) wins[dragging].y = 0;
    }
    
    prevLeft = mouseLeft;
}

void wmDraw()
{
    Color titleCol = {40, 90, 200}, white = {255, 255, 255};
    for (int i = 0; i < winCount; i++)
    {
        Window* w = &wins[i];
        gfxFillRect(w->x, w->y, w->w, TITLE_H, titleCol);
        gfxDrawString(w->title, w->x + 4, w->y + 4, white);
        gfxFillRect(w->x, w->y + TITLE_H, w->w, w->h, w->bg);
    }
}

const char* wmTitle(int index) { return wins[index].title; }
int  wmIsFocused(int index) { return index == winCount - 1; }
void wmFocus(int index) { wmRaise(index); }
