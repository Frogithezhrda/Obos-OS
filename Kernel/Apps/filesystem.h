#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "../Graphics/apps.h"
#include "../Fs/superblock.h"
#include "../SystemLib/obosMemory.h"

#define MAX_FILES 64

void drawFsFiles();
void openFileEditor();

extern Icon fileIconFM;
extern Icon folderIconFM;
extern Icon fileIcons[];
extern int fileCount;
extern int clickedFileIndex;
extern Button deleteIconFM;

#endif