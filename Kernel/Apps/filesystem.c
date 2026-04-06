#include "filesystem.h"

static Label fileLabels[MAX_FILES];
static Icon fileIcons[MAX_FILES];
static FileEntry fileEntries[MAX_FILES];
static int fileCount = 0;

void drawFsFiles()
{
    fileCount = getDirectoryEntries(fileEntries, MAX_FILES);

    for (int i = 0; i < fileCount; i++)
    {
        int xDir = i / 6;
        int yDir = i % 6;

        int baseX = 100 + xDir * 180;
        int baseY = 140 + yDir * 60;

        unsigned char (*iconData)[16] =
            (fileEntries[i].type == Directory) ? folder16 : file16;

        Window iconWin = createWindow(baseX, baseY, 24, 24, (Color){239, 255, 188}, VISIBLE);

        fileIcons[i] = createIcon(&iconWin, iconData, NULL);
        drawIcon(&fileIcons[i], 2);

        Window textWin = createWindow(baseX + 40, baseY + 12, 120, 16, (Color){239, 255, 188}, VISIBLE);

        fileLabels[i] = createLabel(&textWin, fileEntries[i].name, 1, BLACK);
        drawLabel(&fileLabels[i]);
    }
}