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
#define DOUBLE_CLICK_TICKS (TIMER_HZ / 2)
#define DRAG_THRESHOLD 4
#define MAX_ICONS 32

static int cellCol[MAX_ICONS];
static int cellRow[MAX_ICONS];
static int placed = 0;
static int selected = -1;
static int lastClickIcon = -1;
static unsigned int lastClickTick = 0;
static int prevLeft = 0;
static int dragIcon = -1;
static int dragging = 0;
static int dragDX = 0;
static int dragDY = 0;
static int dragStartX = 0;
static int dragStartY = 0;
static int dragX = 0;
static int dragY = 0;

static int iconCount(void)
{
    int n = appCount();
    return n > MAX_ICONS ? MAX_ICONS : n;
}

static int gridCols(void)
{
    int c = (SCREEN_WIDTH - MARGIN) / CELL_W;
    return c < 1 ? 1 : c;
}

static int gridRows(void)
{
    int r = (TASKBAR_Y - MARGIN) / CELL_H;
    return r < 1 ? 1 : r;
}

static int cellX(int col)
{
    return MARGIN + col * CELL_W;
}

static int cellY(int row)
{
    return MARGIN + row * CELL_H;
}

static void placeAll(void)
{
    int rows = gridRows();
    int n = iconCount();
    for (int i = 0; i < n; i++)
    {
        cellCol[i] = i / rows;
        cellRow[i] = i % rows;
    }
    placed = 1;
}

static int cellTaken(int col, int row, int ignore)
{
    int n = iconCount();
    for (int i = 0; i < n; i++)
        if (i != ignore && cellCol[i] == col && cellRow[i] == row) return 1;
    return 0;
}

static void iconPos(int i, int* x, int* y)
{
    if (i == dragIcon && dragging)
    {
        *x = dragX;
        *y = dragY;
        return;
    }
    *x = cellX(cellCol[i]);
    *y = cellY(cellRow[i]);
}

static int iconAt(int px, int py)
{
    int n = iconCount();
    for (int i = 0; i < n; i++)
    {
        int x;
        int y;
        iconPos(i, &x, &y);
        if (px >= x && px < x + CELL_W && py >= y && py < y + CELL_H) return i;
    }
    return -1;
}

static int targetCell(int i, int* outCol, int* outRow)
{
    int cols = gridCols();
    int rows = gridRows();
    int col = (dragX + CELL_W / 2 - MARGIN) / CELL_W;
    int row = (dragY + CELL_H / 2 - MARGIN) / CELL_H;
    if (col < 0) col = 0;
    if (col >= cols) col = cols - 1;
    if (row < 0) row = 0;
    if (row >= rows) row = rows - 1;
    if (cellTaken(col, row, i))
    {
        int bestCol = -1;
        int bestRow = -1;
        int best = 0x7FFFFFFF;
        for (int c = 0; c < cols; c++)
        {
            for (int r = 0; r < rows; r++)
            {
                if (cellTaken(c, r, i)) continue;
                int dc = c - col;
                int dr = r - row;
                int d = dc * dc + dr * dr;
                if (d < best)
                {
                    best = d;
                    bestCol = c;
                    bestRow = r;
                }
            }
        }
        if (bestCol < 0) return 0;
        col = bestCol;
        row = bestRow;
    }
    *outCol = col;
    *outRow = row;
    return 1;
}

void desktopUpdate(void)
{
    browserPoll(getTicks());
    if (!placed) placeAll();
    if (mouseLeft && !prevLeft && mouseY < TASKBAR_Y && !wmConsumedClick())
    {
        int i = iconAt(mouseX, mouseY);
        unsigned int now = getTicks();
        if (i >= 0 && i == lastClickIcon && now - lastClickTick <= DOUBLE_CLICK_TICKS)
        {
            appLaunch(i);
            lastClickIcon = -1;
            dragIcon = -1;
        }
        else
        {
            lastClickIcon = i;
            lastClickTick = now;
            if (i >= 0)
            {
                int ix;
                int iy;
                iconPos(i, &ix, &iy);
                dragIcon = i;
                dragging = 0;
                dragDX = mouseX - ix;
                dragDY = mouseY - iy;
                dragStartX = mouseX;
                dragStartY = mouseY;
            }
        }
        selected = i;
    }
    if (dragIcon >= 0)
    {
        if (!mouseLeft)
        {
            if (dragging)
            {
                int col;
                int row;
                if (targetCell(dragIcon, &col, &row))
                {
                    cellCol[dragIcon] = col;
                    cellRow[dragIcon] = row;
                }
                lastClickIcon = -1;
            }
            dragIcon = -1;
            dragging = 0;
        }
        else
        {
            int dx = mouseX - dragStartX;
            int dy = mouseY - dragStartY;
            if (!dragging && dx * dx + dy * dy > DRAG_THRESHOLD * DRAG_THRESHOLD) dragging = 1;
            if (dragging)
            {
                dragX = mouseX - dragDX;
                dragY = mouseY - dragDY;
                if (dragX < 0) dragX = 0;
                if (dragY < 0) dragY = 0;
                if (dragX > SCREEN_WIDTH - CELL_W) dragX = SCREEN_WIDTH - CELL_W;
                if (dragY > TASKBAR_Y - CELL_H) dragY = TASKBAR_Y - CELL_H;
            }
        }
    }
    prevLeft = mouseLeft;
}

void desktopDraw(void)
{
    if (!placed) placeAll();
    Color white = {255, 255, 255};
    Color hl = {0, 0, 128};
    int n = iconCount();

    if (dragging && dragIcon >= 0)
    {
        int col;
        int row;
        if (targetCell(dragIcon, &col, &row))
        {
            int tx = cellX(col);
            int ty = cellY(row);
            gfxFillRect(tx, ty, CELL_W - 4, 1, white);
            gfxFillRect(tx, ty + CELL_H - 5, CELL_W - 4, 1, white);
            gfxFillRect(tx, ty, 1, CELL_H - 4, white);
            gfxFillRect(tx + CELL_W - 5, ty, 1, CELL_H - 4, white);
        }
    }

    for (int pass = 0; pass < 2; pass++)
    {
        for (int i = 0; i < n; i++)
        {
            int isDragged = (i == dragIcon && dragging) ? 1 : 0;
            if (isDragged != pass) continue;
            int x;
            int y;
            iconPos(i, &x, &y);
            gfxDrawIcon(x + (CELL_W - ICON_PX) / 2, y + 4, appIcon(i), ICON_SCALE);
            const char* name = appName(i);
            int len = strLen(name);
            int lx = x + (CELL_W - len * 8) / 2;
            int ly = y + 4 + ICON_PX + 6;
            if (i == selected) gfxFillRect(lx - 2, ly - 2, len * 8 + 4, 12, hl);
            gfxDrawString(name, lx, ly, white);
        }
    }
}