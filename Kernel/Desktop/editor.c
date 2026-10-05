#include "editor.h"
#include "../Graphics/gfx.h"
#include "../Drivers/keys.h"
#include "../Drivers/timerDriver.h"
#include "../Fs/superblock.h"

#define EDITOR_MAX 8192
#define TOOLBAR_H 20
#define FOOTER_H 14
#define LINE_H 10
#define PAD 4
#define SAVE_X 4
#define SAVE_W 40
#define SAVEAS_X (SAVE_X + SAVE_W + 4)
#define SAVEAS_W 64

static char buf[EDITOR_MAX];
static char fileName[FILE_NAME_LENGTH];
static unsigned int fileDir = 0;
static unsigned int savedDir = 0;
static int hasFile = 0;
static int len = 0;
static int cursor = 0;
static int wantCol = 0;
static int topLine = 0;
static int leftCol = 0;
static int dirty = 0;
static int visibleRows = 1;
static int visibleCols = 1;
static const char* status = 0;
static unsigned int statusUntil = 0;
static int naming = 0;
static int nameLen = 0;
static char nameBuf[FILE_NAME_LENGTH];

static int strLen(const char* s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

static void strAppend(char* dst, const char* src)
{
    int n = strLen(dst);
    while (*src) dst[n++] = *src++;
    dst[n] = 0;
}

static void utoa10(unsigned int v, char* out)
{
    char tmp[12];
    int n = 0;
    if (v == 0) tmp[n++] = '0';
    while (v)
    {
        tmp[n++] = '0' + v % 10;
        v /= 10;
    }
    for (int i = 0; i < n; i++) out[i] = tmp[n - 1 - i];
    out[n] = 0;
}

static void setStatus(const char* msg)
{
    status = msg;
    statusUntil = getTicks() + 2 * TIMER_HZ;
}

static int lineStart(int pos)
{
    while (pos > 0 && buf[pos - 1] != '\n') pos--;
    return pos;
}

static int lineEnd(int pos)
{
    while (pos < len && buf[pos] != '\n') pos++;
    return pos;
}

static void updateWantCol(void)
{
    wantCol = cursor - lineStart(cursor);
}

static void insertChar(char c)
{
    if (len >= EDITOR_MAX)
    {
        setStatus("Buffer is full");
        return;
    }
    for (int i = len; i > cursor; i--) buf[i] = buf[i - 1];
    buf[cursor++] = c;
    len++;
    dirty = 1;
}

static void removeAt(int pos)
{
    for (int i = pos; i < len - 1; i++) buf[i] = buf[i + 1];
    len--;
    dirty = 1;
}

static void moveUp(void)
{
    int start = lineStart(cursor);
    if (start == 0) return;
    int prevEnd = start - 1;
    int prevStart = lineStart(prevEnd);
    int prevLen = prevEnd - prevStart;
    cursor = prevStart + (wantCol < prevLen ? wantCol : prevLen);
}

static void moveDown(void)
{
    int end = lineEnd(cursor);
    if (end >= len) return;
    int nextStart = end + 1;
    int nextLen = lineEnd(nextStart) - nextStart;
    cursor = nextStart + (wantCol < nextLen ? wantCol : nextLen);
}

static void save(void)
{
    if (!hasFile) return;
    savedDir = currentDirINode;
    currentDirINode = fileDir;
    int result = writeFile(fileName, buf, len);
    currentDirINode = savedDir;
    if (result == SUCCESS)
    {
        dirty = 0;
        setStatus("Saved");
        if (fileDir == currentDirINode) filemgrRefresh();
    }
    else
    {
        setStatus("Save failed");
    }
}

static void startSaveAs(void)
{
    if (!hasFile) return;
    strcpy(nameBuf, fileName);
    nameLen = strLen(nameBuf);
    naming = 1;
}

static void finishSaveAs(void)
{
    naming = 0;
    if (nameLen == 0)
    {
        setStatus("Name cannot be empty");
        return;
    }
    if (strcmp(nameBuf, fileName) == 0)
    {
        save();
        return;
    }
    if (strcmp(nameBuf, ".") == 0 || strcmp(nameBuf, "..") == 0)
    {
        setStatus("Name not allowed");
        return;
    }
    savedDir = currentDirINode;
    currentDirINode = fileDir;
    int created = createFile(nameBuf, File);
    int result = ERROR;
    if (created != ERROR) result = writeFile(nameBuf, buf, len);
    currentDirINode = savedDir;
    if (created == ERROR)
    {
        setStatus("Name taken or cannot create");
        return;
    }
    strcpy(fileName, nameBuf);
    if (result == SUCCESS)
    {
        dirty = 0;
        setStatus("Saved as new file");
    }
    else
    {
        setStatus("Save failed");
    }
    if (fileDir == currentDirINode) filemgrRefresh();
}

static void nameKey(int key)
{
    if (key == KEY_ESC)
    {
        naming = 0;
    }
    else if (key == KEY_ENTER)
    {
        finishSaveAs();
    }
    else if (key == KEY_BACKSPACE)
    {
        if (nameLen > 0) nameBuf[--nameLen] = 0;
    }
    else if (key >= 32 && key < 127 && key != '/' && nameLen < FILE_NAME_LENGTH - 1)
    {
        nameBuf[nameLen++] = (char)key;
        nameBuf[nameLen] = 0;
    }
}

const char* editorOpen(const char* name, unsigned int size)
{
    if (hasFile && strcmp(fileName, name) == 0 && fileDir == currentDirINode) return 0;
    if (hasFile && dirty) return "Save the open file first";
    if (size > EDITOR_MAX) return "File too large (8 KB max)";
    if (size > 0 && readFile(name, buf, size) != SUCCESS) return "Read failed";
    strcpy(fileName, name);
    fileDir = currentDirINode;
    len = (int)size;
    cursor = 0;
    wantCol = 0;
    topLine = 0;
    leftCol = 0;
    dirty = 0;
    hasFile = 1;
    return 0;
}

void editorKey(int key)
{
    if (!hasFile) return;
    if (naming)
    {
        nameKey(key);
        return;
    }
    switch (key)
    {
        case KEY_LEFT:
            if (cursor > 0) cursor--;
            updateWantCol();
            break;
        case KEY_RIGHT:
            if (cursor < len) cursor++;
            updateWantCol();
            break;
        case KEY_UP:
            moveUp();
            break;
        case KEY_DOWN:
            moveDown();
            break;
        case KEY_HOME:
            cursor = lineStart(cursor);
            updateWantCol();
            break;
        case KEY_END:
            cursor = lineEnd(cursor);
            updateWantCol();
            break;
        case KEY_BACKSPACE:
            if (cursor > 0)
            {
                removeAt(cursor - 1);
                cursor--;
            }
            updateWantCol();
            break;
        case KEY_DELETE:
            if (cursor < len) removeAt(cursor);
            break;
        case KEY_ENTER:
            insertChar('\n');
            updateWantCol();
            break;
        case KEY_TAB:
            for (int i = 0; i < 4; i++) insertChar(' ');
            updateWantCol();
            break;
        case KEY_SAVE:
            save();
            break;
        case KEY_SAVE_AS:
            startSaveAs();
            break;
        default:
            if (key >= 32 && key < 127)
            {
                insertChar((char)key);
                updateWantCol();
            }
            break;
    }
}

void editorClick(int lx, int ly)
{
    if (naming)
    {
        naming = 0;
        return;
    }
    if (ly < TOOLBAR_H)
    {
        if (ly >= 2 && ly < TOOLBAR_H - 2)
        {
            if (lx >= SAVE_X && lx < SAVE_X + SAVE_W) save();
            else if (lx >= SAVEAS_X && lx < SAVEAS_X + SAVEAS_W) startSaveAs();
        }
        return;
    }
    if (!hasFile) return;
    if (ly >= TOOLBAR_H + 2 + visibleRows * LINE_H) return;
    int line = topLine + (ly - TOOLBAR_H - 2) / LINE_H;
    int col = leftCol + (lx - PAD) / 8;
    int pos = 0;
    for (int l = 0; l < line; l++)
    {
        int end = lineEnd(pos);
        if (end >= len) break;
        pos = end + 1;
    }
    int lineLen = lineEnd(pos) - pos;
    cursor = pos + (col < lineLen ? col : lineLen);
    updateWantCol();
}

static void drawBtn(int x, int y, int w, int h, const char* label, Color textColor)
{
    Color dark = {96, 96, 96};
    gfxFillRect(x, y, w, h, GRAY);
    gfxFillRect(x, y, w, 1, WHITE);
    gfxFillRect(x, y, 1, h, WHITE);
    gfxFillRect(x, y + h - 1, w, 1, dark);
    gfxFillRect(x + w - 1, y, 1, h, dark);
    gfxDrawString(label, x + 4, y + (h - 8) / 2, textColor);
}

void editorDraw(int x, int y, int w, int h)
{
    Color dark = {96, 96, 96};

    gfxFillRect(x, y, w, TOOLBAR_H, GRAY);
    int by = y + 2;
    int bh = TOOLBAR_H - 4;
    drawBtn(x + SAVE_X, by, SAVE_W, bh, "Save", hasFile ? BLACK : dark);
    drawBtn(x + SAVEAS_X, by, SAVEAS_W, bh, "Save As", hasFile ? BLACK : dark);

    if (!hasFile)
    {
        gfxDrawString("No file open. Double-click a file in Files.", x + PAD, y + TOOLBAR_H + 8, BLACK);
        return;
    }

    char title[FILE_NAME_LENGTH + 4];
    title[0] = 0;
    if (dirty) strAppend(title, "* ");
    strAppend(title, fileName);
    int titleX = SAVEAS_X + SAVEAS_W + 8;
    int maxTitle = (w - titleX - PAD) / 8;
    if (maxTitle < 0) maxTitle = 0;
    if (strLen(title) > maxTitle) title[maxTitle] = 0;
    gfxDrawString(title, x + titleX, by + (bh - 8) / 2, BLACK);

    visibleRows = (h - TOOLBAR_H - FOOTER_H - 2) / LINE_H;
    if (visibleRows < 1) visibleRows = 1;
    visibleCols = (w - 2 * PAD) / 8;
    if (visibleCols < 1) visibleCols = 1;

    int curLine = 0;
    for (int i = 0; i < cursor; i++)
        if (buf[i] == '\n') curLine++;
    int curCol = cursor - lineStart(cursor);
    if (curLine < topLine) topLine = curLine;
    if (curLine >= topLine + visibleRows) topLine = curLine - visibleRows + 1;
    if (curCol < leftCol) leftCol = curCol;
    if (curCol >= leftCol + visibleCols) leftCol = curCol - visibleCols + 1;

    int pos = 0;
    for (int l = 0; l < topLine; l++) pos = lineEnd(pos) + 1;
    for (int r = 0; r < visibleRows && pos <= len; r++)
    {
        int end = lineEnd(pos);
        int ry = y + TOOLBAR_H + 2 + r * LINE_H;
        for (int c = leftCol; c < end - pos && c < leftCol + visibleCols; c++)
        {
            char ch = buf[pos + c];
            if (ch < 32 || ch > 126) ch = '?';
            gfxDrawChar(x + PAD + (c - leftCol) * 8, ry, ch, BLACK);
        }
        pos = end + 1;
    }

    if (!naming && (getTicks() / (TIMER_HZ / 2)) % 2 == 0)
        gfxFillRect(x + PAD + (curCol - leftCol) * 8, y + TOOLBAR_H + 2 + (curLine - topLine) * LINE_H, 2, LINE_H - 1, BLACK);

    gfxFillRect(x, y + h - FOOTER_H, w, FOOTER_H, GRAY);
    int maxChars = (w - 2 * PAD) / 8;
    if (naming)
    {
        char prompt[FILE_NAME_LENGTH + 16];
        strcpy(prompt, "Save as: ");
        strAppend(prompt, nameBuf);
        if ((getTicks() / (TIMER_HZ / 2)) % 2 == 0) strAppend(prompt, "_");
        int pl = strLen(prompt);
        gfxDrawString(pl > maxChars ? prompt + (pl - maxChars) : prompt, x + PAD, y + h - FOOTER_H + 3, BLUE);
    }
    else if (status && getTicks() < statusUntil)
    {
        gfxDrawString(status, x + PAD, y + h - FOOTER_H + 3, RED);
    }
    else
    {
        char text[48];
        char num[12];
        strcpy(text, "Ln ");
        utoa10(curLine + 1, num);
        strAppend(text, num);
        strAppend(text, ", Col ");
        utoa10(curCol + 1, num);
        strAppend(text, num);
        strAppend(text, "   ");
        utoa10(len, num);
        strAppend(text, num);
        strAppend(text, "/");
        utoa10(EDITOR_MAX, num);
        strAppend(text, num);
        gfxDrawString(text, x + PAD, y + h - FOOTER_H + 3, BLACK);
    }
}

void editorRenamed(const char* oldName, const char* newName)
{
    if (hasFile && fileDir == currentDirINode && strcmp(fileName, oldName) == 0) strcpy(fileName, newName);
}

void editorMoved(const char* name, unsigned int oldDir, unsigned int newDir)
{
    if (hasFile && fileDir == oldDir && strcmp(fileName, name) == 0) fileDir = newDir;
}