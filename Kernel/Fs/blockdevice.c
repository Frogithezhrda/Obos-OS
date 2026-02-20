#include "blockdevice.h"

int readBlock(const int blockId, Block* block)
{
    return readATABlock(blockId, block);
}

int writeBlock(const int blockId, Block* block)
{
    return writeATABlock(blockId, block);
}