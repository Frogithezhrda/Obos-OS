#include "filemgr.h"
#include "../Fs/superblock.h"
#include "../Graphics/gfx.h"
#include "../Drivers/timerDriver.h"
#include "editor.h"
#include "apps.h"

#define MAX_ENTRIES 64
#define TOOLBAR_H 20
#define PATH_H 12
#define HEADER_H (TOOLBAR_H + PATH_H)
#define FOOTER_H 14
#define ROW_H 12
#define BTN_COUNT 7

static FileEntry raw[MAX_ENTRIES];
static FileEntry entries[MAX_ENTRIES];
static int count = 0;
static int selected = -1;
static int loaded = 0;
static int visibleRows = 0;
static int lastClick = -1;
static unsigned int lastTick = 0;
static char path[128] = "/";
static const char* btnLabel[BTN_COUNT] = {"Up", "Dir", "File", "Ren", "Del", "Move", "X"};
static const char* status = 0;
static unsigned int statusUntil = 0;
static int confirmIdx = -1;
static unsigned int confirmUntil = 0;
static int renaming = 0;
static int renameLen = 0;
static char renameBuf[FILE_NAME_LENGTH];
static char renameOld[FILE_NAME_LENGTH];
static int moving = 0;
static int moveIsDir = 0;
static unsigned int moveSrcDir = 0;
static char moveName[FILE_NAME_LENGTH];
static char moveSrcPath[128];

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

static void pathPush(const char* name)
{
    int len = strLen(path);
    if (len + strLen(name) + 2 >= (int)sizeof(path)) return;
    if (len > 1) path[len++] = '/';
    for (int i = 0; name[i]; i++) path[len++] = name[i];
    path[len] = 0;
}

static void pathUp(void)
{
    int len = strLen(path);
    while (len > 1 && path[len - 1] != '/') len--;
    if (len > 1) len--;
    path[len] = 0;
}

static void refresh(void)
{
    int n = getDirectoryEntries(raw, MAX_ENTRIES);
    count = 0;
    selected = -1;
    lastClick = -1;
    confirmIdx = -1;
    for (int i = 0; i < n; i++)
        if (currentDirINode != 0 && strcmp(raw[i].name, "..") == 0) entries[count++] = raw[i];
    for (int i = 0; i < n; i++)
        if (raw[i].type == Directory && strcmp(raw[i].name, ".") != 0 && strcmp(raw[i].name, "..") != 0)
            entries[count++] = raw[i];
    for (int i = 0; i < n; i++)
        if (raw[i].type != Directory) entries[count++] = raw[i];
}

static int findEntry(const char* name)
{
    for (int i = 0; i < count; i++)
        if (strcmp(entries[i].name, name) == 0) return i;
    return -1;
}
static int startsWith(const char* s, const char* prefix)
{
    while (*prefix)
    {
        if (*s != *prefix) return 0;
        s++;
        prefix++;
    }
    return 1;
}

static void startMove(void)
{
    if (selected < 0 || selected >= count || strcmp(entries[selected].name, "..") == 0)
    {
        setStatus("Select something first");
        return;
    }
    strcpy(moveName, entries[selected].name);
    strcpy(moveSrcPath, path);
    moveIsDir = entries[selected].type == Directory;
    moveSrcDir = currentDirINode;
    moving = 1;
}

static void finishMove(void)
{
    unsigned int dest = currentDirINode;
    if (dest == moveSrcDir)
    {
        setStatus("Already in this folder");
        return;
    }
    if (findEntry(moveName) >= 0)
    {
        setStatus("Name exists in this folder");
        return;
    }
    if (moveIsDir)
    {
        char full[256];
        strcpy(full, moveSrcPath);
        if (strLen(full) > 1) strAppend(full, "/");
        strAppend(full, moveName);
        int n = strLen(full);
        if (startsWith(path, full) && (path[n] == 0 || path[n] == '/'))
        {
            setStatus("Cannot move a folder into itself");
            return;
        }
    }
    currentDirINode = moveSrcDir;
    int result = moveFile(moveName, dest);
    currentDirINode = dest;
    moving = 0;
    if (result != SUCCESS)
    {
        setStatus("Move failed");
        return;
    }
    editorMoved(moveName, moveSrcDir, dest);
    refresh();
    int i = findEntry(moveName);
    if (i >= 0) selected = i;
}

static void startRename(void)
{
    if (selected < 0 || selected >= count || strcmp(entries[selected].name, "..") == 0)
    {
        setStatus("Select something first");
        return;
    }
    strcpy(renameOld, entries[selected].name);
    strcpy(renameBuf, entries[selected].name);
    renameLen = strLen(renameBuf);
    renaming = 1;
}
static void finishRename(void)
{
    renaming = 0;
    if (renameLen == 0)
    {
        setStatus("Name cannot be empty");
        return;
    }
    if (strcmp(renameBuf, renameOld) == 0) return;
    if (strcmp(renameBuf, ".") == 0 || strcmp(renameBuf, "..") == 0 || findEntry(renameBuf) >= 0)
    {
        setStatus("Name not allowed or taken");
        return;
    }
    if (renameFile(renameOld, renameBuf) != SUCCESS)
    {
        setStatus("Rename failed");
        return;
    }
    editorRenamed(renameOld, renameBuf);
    refresh();
    int i = findEntry(renameBuf);
    if (i >= 0) selected = i;
}

void filemgrKey(int key)
{
    if (moving && key == KEY_ESC)
    {
        moving = 0;
        return;
    }
    if (!renaming) return;
    if (key == KEY_ESC)
    {
        renaming = 0;
    }
    else if (key == KEY_ENTER)
    {
        finishRename();
    }
    else if (key == KEY_BACKSPACE)
    {
        if (renameLen > 0) renameBuf[--renameLen] = 0;
    }
    else if (key >= 32 && key < 127 && key != '/' && renameLen < FILE_NAME_LENGTH - 1)
    {
        renameBuf[renameLen++] = (char)key;
        renameBuf[renameLen] = 0;
    }
}
static void goToRoot(void)
{
    for (int guard = 64; currentDirINode != 0 && guard > 0; guard--) cd("..");
}

static void navigate(const char* name)
{
    if (cd(name) != SUCCESS) return;
    if (strcmp(name, "..") == 0) pathUp();
    else pathPush(name);
    refresh();
}

static void uniqueName(const char* base, char* out)
{
    strcpy(out, base);
    for (int k = 2; findEntry(out) >= 0 && k < 1000; k++)
    {
        char num[12];
        strcpy(out, base);
        utoa10(k, num);
        strAppend(out, num);
    }
}

static void createEntry(int isDir)
{
    char name[24];
    uniqueName(isDir ? "NewDir" : "NewFile", name);
    if (isDir) createDir(name);
    else createFile(name, File);
    refresh();
    int i = findEntry(name);
    if (i < 0) setStatus("Create failed");
    else selected = i;
}

static void deleteSelected(void)
{
    if (selected < 0 || selected >= count)
    {
        setStatus("Select something first");
        return;
    }
    if (strcmp(entries[selected].name, "..") == 0) return;
    // deleteFile doesnt recurse, so only empty folders are allowed
    if (entries[selected].type == Directory && entries[selected].size > 2 * DIR_ENTRY_SIZE)
    {
        setStatus("Folder is not empty");
        return;
    }
    unsigned int now = getTicks();
    if (confirmIdx != selected || now >= confirmUntil)
    {
        confirmIdx = selected;
        confirmUntil = now + 2 * TIMER_HZ;
        setStatus("Press Del again to confirm");
        return;
    }
    char target[FILE_NAME_LENGTH];
    strcpy(target, entries[selected].name);
    deleteFile(target);
    refresh();
}

static int btnCount(void)
{
    return moving ? 7 : 6;
}

static const char* btnText(int i)
{
    if (i == 5 && moving) return "Here";
    return btnLabel[i];
}

static int btnW(int i)
{
    return strLen(btnText(i)) * 8 + 8;
}

static int btnX(int i)
{
    int x = 4;
    for (int k = 0; k < i; k++) x += btnW(k) + 4;
    return x;
}

static void doToolbar(int i)
{
    if (moving && i != 0 && i != 5 && i != 6) return;
    switch (i)
    {
        case 0:
            if (currentDirINode != 0) navigate("..");
            break;
        case 1:
            createEntry(1);
            break;
        case 2:
            createEntry(0);
            break;
        case 3:
            startRename();
            break;
        case 4:
            deleteSelected();
            break;
        case 5:
            if (moving) finishMove();
            else startMove();
            break;
        case 6:
            moving = 0;
            break;
    }
}

void filemgrClick(int lx, int ly)
{

    if (renaming)
    {
        renaming = 0;
        return;
    }
    if (ly < TOOLBAR_H)
    {
        for (int i = 0; i < btnCount(); i++)
        {
            if (lx >= btnX(i) && lx < btnX(i) + btnW(i) && ly >= 2 && ly < TOOLBAR_H - 2)
            {
                doToolbar(i);
                return;
            }
        }
        return;
    }
    if (ly < HEADER_H) return;
    int i = (ly - HEADER_H) / ROW_H;
    if (i < 0 || i >= visibleRows || i >= count)
    {
        selected = -1;
        lastClick = -1;
        return;
    }
    unsigned int now = getTicks();
    if (i == lastClick && now - lastTick <= TIMER_HZ / 2)
    {
        if (entries[i].type == Directory)
        {
            char target[FILE_NAME_LENGTH];
            strcpy(target, entries[i].name);
            navigate(target);
        }
        else if (!moving)
        {
            const char* err = editorOpen(entries[i].name, entries[i].size);
            if (err) setStatus(err);
            else appLaunchByName("Editor");
        }
        lastClick = -1;
        return;
    }
    selected = i;
    lastClick = i;
    lastTick = now;
}

static void drawBtn(int x, int y, int w, int h, const char* label, Color textColor)
{
    Color gray = {192, 192, 192};
    Color light = {255, 255, 255};
    Color dark = {96, 96, 96};
    gfxFillRect(x, y, w, h, gray);
    gfxFillRect(x, y, w, 1, light);
    gfxFillRect(x, y, 1, h, light);
    gfxFillRect(x, y + h - 1, w, 1, dark);
    gfxFillRect(x + w - 1, y, 1, h, dark);
    gfxDrawString(label, x + 4, y + (h - 8) / 2, textColor);
}

void filemgrDraw(int x, int y, int w, int h)
{
    if (!loaded)
    {
        goToRoot();
        refresh();
        loaded = 1;
    }
    Color dark = {96, 96, 96};
    Color strip = {230, 230, 230};
    Color hl = {0, 0, 128};

    gfxFillRect(x, y, w, TOOLBAR_H, GRAY);
    for (int i = 0; i < btnCount(); i++)
    {
        int off = (i == 0 && currentDirINode == 0) || (moving && i != 0 && i != 5 && i != 6);
        drawBtn(x + btnX(i), y + 2, btnW(i), TOOLBAR_H - 4, btnText(i), off ? dark : BLACK);
    }

    gfxFillRect(x, y + TOOLBAR_H, w, PATH_H, strip);
    int maxChars = (w - 8) / 8;
    if (renaming)
    {
        char prompt[FILE_NAME_LENGTH + 16];
        strcpy(prompt, "Rename: ");
        strAppend(prompt, renameBuf);
        if ((getTicks() / (TIMER_HZ / 2)) % 2 == 0) strAppend(prompt, "_");
        int pl = strLen(prompt);
        gfxFillRect(x, y + TOOLBAR_H, w, PATH_H, WHITE);
        gfxDrawString(pl > maxChars ? prompt + (pl - maxChars) : prompt, x + 4, y + TOOLBAR_H + 2, BLACK);
    }
    else
    {
        int plen = strLen(path);
        gfxDrawString(plen > maxChars ? path + (plen - maxChars) : path, x + 4, y + TOOLBAR_H + 2, BLACK);
    }

    visibleRows = (h - HEADER_H - FOOTER_H) / ROW_H;
    if (visibleRows < 0) visibleRows = 0;
    for (int i = 0; i < count && i < visibleRows; i++)
    {
        int ry = y + HEADER_H + i * ROW_H;
        Color c = (i == selected) ? WHITE : BLACK;
        if (i == selected) gfxFillRect(x, ry, w, ROW_H, hl);
        gfxDrawString(entries[i].type == Directory ? "[D]" : "   ", x + 4, ry + 2, c);
        gfxDrawString(entries[i].name, x + 4 + 4 * 8, ry + 2, c);
        if (entries[i].type != Directory)
        {
            char buf[12];
            utoa10(entries[i].size, buf);
            gfxDrawString(buf, x + w - strLen(buf) * 8 - 8, ry + 2, c);
        }
    }

    gfxFillRect(x, y + h - FOOTER_H, w, FOOTER_H, GRAY);
    if (status && getTicks() < statusUntil)
    {
        gfxDrawString(status, x + 4, y + h - FOOTER_H + 3, RED);
    }    
    else if (moving)
    {
        char text[FILE_NAME_LENGTH + 16];
        strcpy(text, "Moving: ");
        strAppend(text, moveName);
        int tl = strLen(text);
        gfxDrawString(tl > maxChars ? text + (tl - maxChars) : text, x + 4, y + h - FOOTER_H + 3, hl);
    }
    else
    {
        char text[48];
        char num[12];
        strcpy(text, "Free: ");
        utoa10((getFreeBlocksCount() * BLOCK_SIZE) / 1024 / 1024, num);
        strAppend(text, num);
        strAppend(text, " MB of ");
        utoa10((TOTAL_BLOCKS * BLOCK_SIZE) / 1024 / 1024, num);
        strAppend(text, num);
        strAppend(text, " MB");
        gfxDrawString(text, x + 4, y + h - FOOTER_H + 3, BLACK);
    }
}
void filemgrRefresh(void)
{
    if (!loaded) return;
    char keep[FILE_NAME_LENGTH];
    int had = selected >= 0 && selected < count;
    if (had) strcpy(keep, entries[selected].name);
    refresh();
    if (had)
    {
        int i = findEntry(keep);
        if (i >= 0) selected = i;
    }
}