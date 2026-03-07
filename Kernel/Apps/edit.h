#ifndef EDIT_H
#define EDIT_H

#include "../Fs/superblock.h"
#include "../Memory/heap.h"
#include "../Drivers/keyboardDriver.h"

void editFile(const char* filename);
void saveFile(const char* filename);
void closeFile();

#endif // EDIT_H