#include "wm.h"
#include "taskbar.h"
#define MAX_WINDOWS 16
#define TITLE_H 16
#define CLOSE_SZ 12
#define CLOSE_PAD 2


typedef struct { int x, y, w, h; const char* title; Color bg; int id; DrawFn draw; ClickFn click; KeyFn key; } Window;

static int clickConsumed = 0;
static int nextId = 0;
static Window wins[MAX_WINDOWS];
static int winCount = 0;
static int dragging = -1, dragDX, dragDY, prevLeft = 0;

int wmNextId(void) { return nextId; }
int wmConsumedClick(void) { return clickConsumed; }

void wmSetClick(int id, ClickFn fn)
{
    int i = wmIndexOfId(id);
    if (i >= 0) wins[i].click = fn;
}

static void wmClose(int index)
{
    for (int k = index; k < winCount - 1; k++) wins[k] = wins[k + 1];
    winCount--;
    dragging = -1;
}

void wmCloseId(int id)
{
    int idx = wmIndexOfId(id);
    if (idx >= 0) wmClose(idx);
}

static int onCloseButton(const Window* w, int px, int py)
{
    int bx = w->x + w->w - CLOSE_SZ - CLOSE_PAD;
    int by = w->y + CLOSE_PAD;
    return px >= bx && px < bx + CLOSE_SZ && py >= by && py < by + CLOSE_SZ;
}

static void drawCloseButton(const Window* w)
{
    Color gray = {192, 192, 192}, light = {255, 255, 255};
    Color dark = {96, 96, 96}, black = {0, 0, 0};
    int bx = w->x + w->w - CLOSE_SZ - CLOSE_PAD;
    int by = w->y + CLOSE_PAD;

    gfxFillRect(bx, by, CLOSE_SZ, CLOSE_SZ, gray);
    gfxFillRect(bx, by, CLOSE_SZ, 1, light);
    gfxFillRect(bx, by, 1, CLOSE_SZ, light);
    gfxFillRect(bx, by + CLOSE_SZ - 1, CLOSE_SZ, 1, dark);
    gfxFillRect(bx + CLOSE_SZ - 1, by, 1, CLOSE_SZ, dark);

    for (int i = 0; i < 6; i++)
    {
        gfxPutPixel(bx + 3 + i, by + 3 + i, black);
        gfxPutPixel(bx + 8 - i, by + 3 + i, black);
    }
}

int wmIndexOfId(int id)
{
    for (int i = 0; i < winCount; i++) if (wins[i].id == id) return i;
    return -1;
}


int wmAddWindow(int x, int y, int w, int h, const char* title, Color bg, DrawFn draw)
{
    if (winCount >= MAX_WINDOWS) return -1;
    wins[winCount++] = (Window){x, y, w, h, title, bg, nextId, draw};
    return nextId++;
}

static void wmRaise(int i)
{
    Window w = wins[i];
    for (int k = i; k < winCount - 1; k++) wins[k] = wins[k + 1];
    wins[winCount - 1] = w;
}

int wmHitTest(int px, int py)
{
    for (int i = winCount - 1; i >= 0; i--)
        if (px >= wins[i].x && px < wins[i].x + wins[i].w &&
            py >= wins[i].y && py < wins[i].y + wins[i].h + TITLE_H)
            return i;
    return -1;
}

void wmUpdate()
{
    clickConsumed = 0;

    if (dragging < 0 && mouseY >= TASKBAR_Y) { prevLeft = mouseLeft; return; }

    if (mouseLeft && !prevLeft)
    {
        int i = wmHitTest(mouseX, mouseY);
        if (i >= 0)
        {
            clickConsumed = 1;
            wmRaise(i);
            i = winCount - 1;

            if (onCloseButton(&wins[i], mouseX, mouseY))
            {
                wmClose(i);
            }
            else if (mouseY < wins[i].y + TITLE_H)
            {
                dragging = i;
                dragDX = mouseX - wins[i].x;
                dragDY = mouseY - wins[i].y;
            }
            else if (wins[i].click)
            {
                wins[i].click(mouseX - wins[i].x, mouseY - wins[i].y - TITLE_H);
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
        drawCloseButton(w);
        gfxFillRect(w->x, w->y + TITLE_H, w->w, w->h, w->bg);
        if (w->draw) w->draw(w->x, w->y + TITLE_H, w->w, w->h);
    }
}

const char* wmTitle(int index) { return wins[index].title; }
int  wmIsFocused(int index) { return index == winCount - 1; }
void wmFocus(int index) { wmRaise(index); }
void wmSetKey(int id, KeyFn fn)
{
    int i = wmIndexOfId(id);
    if (i >= 0) wins[i].key = fn;
}

void wmKey(int key)
{
    if (winCount > 0 && wins[winCount - 1].key) wins[winCount - 1].key(key);
}