#include "blockdevice.h"

int readBlock(const int blockId, Block* block)
{
    if (readATABlock(blockId, block) != SUCCESS) return ERROR;
    return SUCCESS;
}

int writeBlock(const int blockId, Block* block)
{
    if (writeATABlock(blockId, block) != SUCCESS) return ERROR;
    return SUCCESS;
}