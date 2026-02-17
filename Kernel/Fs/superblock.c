#include "superblock.h"

void initializeSuperBlock()
{
    SuperBlock sb;
    sb.magicNumber = MAGIC_NUMBER;

    sb.inodeTableStartBlock = SUPERBLOCK_BLOCK + 1;
    sb.inodeTableBlocks = (sizeof(INodeTable) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    sb.freeBlocksCount = TOTAL_BLOCKS - (sb.inodeTableStartBlock + sb.inodeTableBlocks);

    Block block;
    memset(&block, 0, sizeof(Block));
    memcpy(block.block, &sb, sizeof(SuperBlock));
    if(writeBlock(SUPERBLOCK_BLOCK, &block) == ERROR)
    {
        kernelPanic("File System failed to initialize!");
    }

    INodeTable* table = (INodeTable*)kmalloc(sizeof(INodeTable));
    memset(table, 0, sizeof(INodeTable));
    //this creates the superblock basically...
    for (unsigned int i = 0; i < sb.inodeTableBlocks; i++)
    {
        memset(&block, 0, sizeof(Block));
        memcpy(block.block, ((char*)table) + i * BLOCK_SIZE, BLOCK_SIZE);
        writeBlock(sb.inodeTableStartBlock + i, &block);
    }
}