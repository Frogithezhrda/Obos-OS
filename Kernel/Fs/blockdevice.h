#ifndef BLOCKDEVICE_H
#define BLOCKDEVICE_H

#include "../Drivers/ata.h"

/*
both read and write blocks
they return success or failed
*/
int readBlock(const int blockId, Block* block);
int writeBlock(const int blockId, Block* block);


#endif