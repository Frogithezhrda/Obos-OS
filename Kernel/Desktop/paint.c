#include "paint.h"
#include "../Graphics/gfx.h"
#include "../Drivers/keys.h"
#include "../Drivers/timerDriver.h"
#include "../Fs/superblock.h"
#include "../Memory/heap.h"

#define TOOLBAR_H 20
#define FOOTER_H 14
#define PANEL_W 64
#define CW 160
#define CH 120
#define SCALE 2
#define CANVAS_X (PANEL_W + 8)
#define CANVAS_Y (TOOLBAR_H + 8)
#define TOOL_Y 4
#define BRUSH_Y 98
#define PAL_Y 122
#define NAME_MAX 24
#define BMP_SIZE (54 + 1024 + CW * CH)

#define PAINT_SAVE_DIR currentDirINode
#define NAME_LIM (FILE_NAME_LENGTH - 5 < NAME_MAX - 1 ? FILE_NAME_LENGTH - 5 : NAME_MAX - 1)

void filemgrRefresh(void);

enum
{
    toolPen,
    toolEraser,
    toolFill,
    toolLine,
    toolRect,
    toolCount
};

static const char* toolNames[toolCount] = {
    "Pen", "Eraser", "Fill", "Line", "Rect"
};

static const int brushSizes[3] = {1, 2, 4};

static const Color palette[16] = {
    {0, 0, 0}, {255, 255, 255}, {128, 128, 128}, {192, 192, 192},
    {220, 30, 30}, {255, 140, 0}, {250, 220, 0}, {0, 200, 0},
    {0, 110, 40}, {0, 200, 200}, {0, 80, 220}, {0, 0, 128},
    {130, 40, 170}, {230, 40, 200}, {120, 70, 30}, {255, 150, 170}
};

static unsigned char* canvas;
static unsigned char* backup;
static unsigned short* fillStack;
static unsigned char* bmpBuf;

static int inited;
static int tool;
static int color;
static int brushIdx;
static int drawing;
static int startX;
static int startY;
static int lastX;
static int lastY;
static int nameFocus;
static char nameBuf[NAME_MAX] = "logo";
static unsigned int nameLen = 4;
static char statusBuf[48];
static unsigned int statusUntil;

static unsigned int pLen(const char* s)
{
    unsigned int n = 0;
    while (s[n])
    {
        n++;
    }
    return n;
}

static void pCopy(char* dst, unsigned int max, const char* s)
{
    unsigned int n = 0;
    while (*s && n < max - 1)
    {
        dst[n++] = *s++;
    }
    dst[n] = '\0';
}

static void setStatus(const char* msg)
{
    pCopy(statusBuf, sizeof(statusBuf), msg);
    statusUntil = getTicks() + 3 * TIMER_HZ;
}

static void clearCanvas(void)
{
    for (int i = 0; i < CW * CH; i++)
    {
        canvas[i] = 1;
    }
}

static int initPaint(void)
{
    if (inited)
    {
        return 1;
    }
    canvas = kmalloc(CW * CH);
    backup = kmalloc(CW * CH);
    fillStack = kmalloc(CW * CH * sizeof(unsigned short));
    bmpBuf = kmalloc(BMP_SIZE);
    if (!canvas || !backup || !fillStack || !bmpBuf)
    {
        kfree(canvas);
        kfree(backup);
        kfree(fillStack);
        kfree(bmpBuf);
        canvas = 0;
        backup = 0;
        fillStack = 0;
        bmpBuf = 0;
        return 0;
    }
    inited = 1;
    clearCanvas();
    return 1;
}

static void putPixel(int x, int y, int c)
{
    if (x < 0 || y < 0 || x >= CW || y >= CH)
    {
        return;
    }
    canvas[y * CW + x] = (unsigned char)c;
}

static void stamp(int x, int y, int c)
{
    int b = brushSizes[brushIdx];
    int off = b / 2;
    for (int dy = 0; dy < b; dy++)
    {
        for (int dx = 0; dx < b; dx++)
        {
            putPixel(x - off + dx, y - off + dy, c);
        }
    }
}

static void drawLine(int x0, int y0, int x1, int y1, int c)
{
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int dy = y1 > y0 ? y1 - y0 : y0 - y1;
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    for (;;)
    {
        int e2;
        stamp(x0, y0, c);
        if (x0 == x1 && y0 == y1)
        {
            break;
        }
        e2 = err * 2;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

static void drawRectOutline(int x0, int y0, int x1, int y1, int c)
{
    drawLine(x0, y0, x1, y0, c);
    drawLine(x1, y0, x1, y1, c);
    drawLine(x1, y1, x0, y1, c);
    drawLine(x0, y1, x0, y0, c);
}

static void floodFill(int x, int y, int c)
{
    static const int dxs[4] = {1, -1, 0, 0};
    static const int dys[4] = {0, 0, 1, -1};
    unsigned char old;
    int top = 0;
    if (x < 0 || y < 0 || x >= CW || y >= CH)
    {
        return;
    }
    old = canvas[y * CW + x];
    if (old == c)
    {
        return;
    }
    canvas[y * CW + x] = (unsigned char)c;
    fillStack[top++] = (unsigned short)(y * CW + x);
    while (top > 0)
    {
        int idx = fillStack[--top];
        int px = idx % CW;
        int py = idx / CW;
        for (int k = 0; k < 4; k++)
        {
            int nx = px + dxs[k];
            int ny = py + dys[k];
            if (nx < 0 || ny < 0 || nx >= CW || ny >= CH)
            {
                continue;
            }
            if (canvas[ny * CW + nx] == old)
            {
                canvas[ny * CW + nx] = (unsigned char)c;
                fillStack[top++] = (unsigned short)(ny * CW + nx);
            }
        }
    }
}

static void put16(unsigned char* p, unsigned int v)
{
    p[0] = (unsigned char)v;
    p[1] = (unsigned char)(v >> 8);
}

static void put32(unsigned char* p, unsigned int v)
{
    p[0] = (unsigned char)v;
    p[1] = (unsigned char)(v >> 8);
    p[2] = (unsigned char)(v >> 16);
    p[3] = (unsigned char)(v >> 24);
}

static void buildBmp(void)
{
    unsigned int pos = 54;
    for (unsigned int i = 0; i < BMP_SIZE; i++)
    {
        bmpBuf[i] = 0;
    }
    bmpBuf[0] = 'B';
    bmpBuf[1] = 'M';
    put32(bmpBuf + 2, BMP_SIZE);
    put32(bmpBuf + 10, 54 + 1024);
    put32(bmpBuf + 14, 40);
    put32(bmpBuf + 18, CW);
    put32(bmpBuf + 22, CH);
    put16(bmpBuf + 26, 1);
    put16(bmpBuf + 28, 8);
    put32(bmpBuf + 34, CW * CH);
    put32(bmpBuf + 38, 2835);
    put32(bmpBuf + 42, 2835);
    put32(bmpBuf + 46, 256);
    for (int i = 0; i < 16; i++)
    {
        bmpBuf[pos++] = palette[i].b;
        bmpBuf[pos++] = palette[i].g;
        bmpBuf[pos++] = palette[i].r;
        bmpBuf[pos++] = 0;
    }
    pos = 54 + 1024;
    for (int y = CH - 1; y >= 0; y--)
    {
        for (int x = 0; x < CW; x++)
        {
            bmpBuf[pos++] = canvas[y * CW + x];
        }
    }
}

static void doSave(void)
{
    char fileName[FILE_NAME_LENGTH];
    unsigned int n;
    unsigned int savedDir;
    int result;
    if (nameLen == 0)
    {
        pCopy(fileName, sizeof(fileName), "logo");
    }
    else
    {
        pCopy(fileName, sizeof(fileName), nameBuf);
    }
    n = pLen(fileName);
    if (n < 4 || fileName[n - 4] != '.' || fileName[n - 3] != 'b' || fileName[n - 2] != 'm' || fileName[n - 1] != 'p')
    {
        fileName[n++] = '.';
        fileName[n++] = 'b';
        fileName[n++] = 'm';
        fileName[n++] = 'p';
        fileName[n] = '\0';
    }
    buildBmp();
    savedDir = currentDirINode;
    currentDirINode = PAINT_SAVE_DIR;
    createFile(fileName, File);
    result = writeFile(fileName, (char*)bmpBuf, BMP_SIZE);
    currentDirINode = savedDir;
    if (result == SUCCESS)
    {
        setStatus("Saved");
        filemgrRefresh();
    }
    else
    {
        setStatus("Save failed");
    }
}

static void drawBtn(int x, int y, int w, int h, const char* label, int selected)
{
    Color gray = {192, 192, 192};
    Color light = {255, 255, 255};
    Color dark = {96, 96, 96};
    Color blue = {0, 0, 128};
    Color black = {0, 0, 0};
    gfxFillRect(x, y, w, h, selected ? blue : gray);
    gfxFillRect(x, y, w, 1, light);
    gfxFillRect(x, y, 1, h, light);
    gfxFillRect(x, y + h - 1, w, 1, dark);
    gfxFillRect(x + w - 1, y, 1, h, dark);
    gfxDrawString(label, x + 4, y + (h - 8) / 2, selected ? light : black);
}

void paintDraw(int x, int y, int w, int h)
{
    Color gray = {192, 192, 192};
    Color white = {255, 255, 255};
    Color black = {0, 0, 0};
    Color dark = {96, 96, 96};
    Color red = {180, 0, 0};
    Color blue = {0, 0, 128};
    int ox = x + CANVAS_X;
    int oy = y + CANVAS_Y;
    int nameChars = 0;
    if (!initPaint())
    {
        gfxFillRect(x, y, w, h, gray);
        gfxDrawString("Out of memory", x + 8, y + 8, black);
        return;
    }
    gfxFillRect(x, y, w, h, gray);
    drawBtn(x + 4, y + 2, 36, 16, "New", 0);
    drawBtn(x + 44, y + 2, 36, 16, "Save", 0);
    gfxFillRect(x + 88, y + 2, 8 * NAME_MAX + 8, 16, white);
    gfxFillRect(x + 88, y + 2, 8 * NAME_MAX + 8, 1, nameFocus ? blue : dark);
    gfxFillRect(x + 88, y + 17, 8 * NAME_MAX + 8, 1, nameFocus ? blue : dark);
    gfxFillRect(x + 88, y + 2, 1, 16, nameFocus ? blue : dark);
    gfxFillRect(x + 88 + 8 * NAME_MAX + 7, y + 2, 1, 16, nameFocus ? blue : dark);
    gfxDrawString(nameBuf, x + 92, y + 6, black);
    if (nameFocus)
    {
        nameChars = (int)nameLen;
        gfxFillRect(x + 92 + nameChars * 8, y + 5, 2, 10, black);
    }
    for (int i = 0; i < toolCount; i++)
    {
        drawBtn(x + 4, y + TOOLBAR_H + TOOL_Y + i * 18, PANEL_W - 8, 16, toolNames[i], i == tool);
    }
    for (int i = 0; i < 3; i++)
    {
        char lbl[2];
        lbl[0] = (char)('0' + brushSizes[i]);
        lbl[1] = '\0';
        drawBtn(x + 4 + i * 20, y + TOOLBAR_H + BRUSH_Y, 18, 16, lbl, i == brushIdx);
    }
    for (int i = 0; i < 16; i++)
    {
        int sx = x + 4 + (i % 2) * 28;
        int sy = y + TOOLBAR_H + PAL_Y + (i / 2) * 12;
        gfxFillRect(sx, sy, 28, 12, palette[i]);
        if (i == color)
        {
            gfxFillRect(sx, sy, 28, 1, white);
            gfxFillRect(sx, sy + 11, 28, 1, white);
            gfxFillRect(sx, sy, 1, 12, white);
            gfxFillRect(sx + 27, sy, 1, 12, white);
            gfxFillRect(sx + 1, sy + 1, 26, 1, black);
            gfxFillRect(sx + 1, sy + 10, 26, 1, black);
            gfxFillRect(sx + 1, sy + 1, 1, 10, black);
            gfxFillRect(sx + 26, sy + 1, 1, 10, black);
        }
    }
    gfxFillRect(ox - 1, oy - 1, CW * SCALE + 2, CH * SCALE + 2, black);
    for (int row = 0; row < CH; row++)
    {
        int col = 0;
        while (col < CW)
        {
            int start = col;
            unsigned char c = canvas[row * CW + col];
            while (col < CW && canvas[row * CW + col] == c)
            {
                col++;
            }
            gfxFillRect(ox + start * SCALE, oy + row * SCALE, (col - start) * SCALE, SCALE, palette[c]);
        }
    }
    gfxFillRect(x, y + h - FOOTER_H, w, FOOTER_H, gray);
    if (statusBuf[0] && getTicks() < statusUntil)
    {
        gfxDrawString(statusBuf, x + 4, y + h - FOOTER_H + 3, red);
    }
    else
    {
        gfxDrawString("Paint 160x120", x + 4, y + h - FOOTER_H + 3, black);
    }
}

static void toCanvas(int lx, int ly, int* cx, int* cy)
{
    int px = (lx - CANVAS_X) / SCALE;
    int py = (ly - CANVAS_Y) / SCALE;
    if (lx < CANVAS_X)
    {
        px = 0;
    }
    if (ly < CANVAS_Y)
    {
        py = 0;
    }
    if (px < 0)
    {
        px = 0;
    }
    if (py < 0)
    {
        py = 0;
    }
    if (px >= CW)
    {
        px = CW - 1;
    }
    if (py >= CH)
    {
        py = CH - 1;
    }
    *cx = px;
    *cy = py;
}

void paintClick(int lx, int ly)
{
    int px;
    int py;
    if (!initPaint())
    {
        return;
    }
    nameFocus = 0;
    if (ly < TOOLBAR_H)
    {
        if (ly < 2 || ly >= TOOLBAR_H - 2)
        {
            return;
        }
        if (lx >= 4 && lx < 40)
        {
            clearCanvas();
            setStatus("New canvas");
        }
        else if (lx >= 44 && lx < 80)
        {
            doSave();
        }
        else if (lx >= 88 && lx < 88 + 8 * NAME_MAX + 8)
        {
            nameFocus = 1;
        }
        return;
    }
    if (lx < PANEL_W)
    {
        int rel = ly - TOOLBAR_H;
        if (rel >= TOOL_Y && rel < TOOL_Y + toolCount * 18)
        {
            tool = (rel - TOOL_Y) / 18;
        }
        else if (rel >= BRUSH_Y && rel < BRUSH_Y + 16 && lx >= 4)
        {
            int idx = (lx - 4) / 20;
            if (idx < 3 && (lx - 4) % 20 < 18)
            {
                brushIdx = idx;
            }
        }
        else if (rel >= PAL_Y && rel < PAL_Y + 96 && lx >= 4 && lx < 60)
        {
            color = ((rel - PAL_Y) / 12) * 2 + (lx - 4) / 28;
        }
        return;
    }
    if (lx < CANVAS_X || ly < CANVAS_Y || lx >= CANVAS_X + CW * SCALE || ly >= CANVAS_Y + CH * SCALE)
    {
        return;
    }
    toCanvas(lx, ly, &px, &py);
    if (tool == toolFill)
    {
        floodFill(px, py, color);
        return;
    }
    for (int i = 0; i < CW * CH; i++)
    {
        backup[i] = canvas[i];
    }
    drawing = 1;
    startX = px;
    startY = py;
    lastX = px;
    lastY = py;
    stamp(px, py, tool == toolEraser ? 1 : color);
}

void paintDrag(int lx, int ly)
{
    int px;
    int py;
    if (!drawing)
    {
        return;
    }
    toCanvas(lx, ly, &px, &py);
    if (tool == toolPen || tool == toolEraser)
    {
        drawLine(lastX, lastY, px, py, tool == toolEraser ? 1 : color);
        lastX = px;
        lastY = py;
        return;
    }
    for (int i = 0; i < CW * CH; i++)
    {
        canvas[i] = backup[i];
    }
    if (tool == toolLine)
    {
        drawLine(startX, startY, px, py, color);
    }
    else
    {
        drawRectOutline(startX, startY, px, py, color);
    }
}

void paintRelease(void)
{
    drawing = 0;
}

void paintKey(int key)
{
    if (!nameFocus)
    {
        return;
    }
    if (key == KEY_ESC)
    {
        nameFocus = 0;
    }
    else if (key == KEY_ENTER)
    {
        nameFocus = 0;
        doSave();
    }
    else if (key == KEY_BACKSPACE)
    {
        if (nameLen > 0)
        {
            nameBuf[--nameLen] = '\0';
        }
    }
    else if (nameLen < NAME_LIM && ((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9') || key == '-' || key == '_' || key == '.'))
    {
        nameBuf[nameLen++] = (char)key;
        nameBuf[nameLen] = '\0';
    }
}