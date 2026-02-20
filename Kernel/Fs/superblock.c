#include "superblock.h"

SuperBlock* sb = NULL;

void createSuperBlock()
{
    printLine("Creating SuperBlock...", BLUE);


    sb->magicNumber = MAGIC_NUMBER;
    sb->inodeTableStartBlock = SUPERBLOCK_BLOCK + 1;
    sb->inodeTableBlocks = (sizeof(INodeTable) / BLOCK_SIZE);
    sb->bitmapStartBlock = sb->inodeTableStartBlock + sb->inodeTableBlocks;
    sb->bitmapBlocks = ((TOTAL_BLOCKS / 8 + BLOCK_SIZE - 1) / BLOCK_SIZE);
    sb->freeBlocksCount = TOTAL_BLOCKS - (SUPERBLOCK_BLOCK + sb->inodeTableBlocks + sb->bitmapBlocks);
    Block* tempBlock = (Block*)kmalloc(sizeof(Block));
    memset(tempBlock, 0, sizeof(Block));
    memcpy(tempBlock->block, sb, sizeof(SuperBlock));
    if(writeBlock(SUPERBLOCK_BLOCK, tempBlock) != SUCCESS)
    {
        kernelPanic("Failed to write superblock to disk!");
    }
    kfree(tempBlock);
    initializeBitmap((unsigned char*)kmalloc(sb->bitmapBlocks * BLOCK_SIZE));
    initializeINodeMap();
    printLine("SuperBlock created successfully!", GREEN);
}

void loadSuperBlock()
{
    printLine("Loading SuperBlock...", BLUE);
    sb = (SuperBlock*)kmalloc(sizeof(SuperBlock));

    if (sb == NULL)
    {
        kernelPanic("Failed to allocate superblock!");
    }

    Block* tempBlock = (Block*)kmalloc(sizeof(Block));
    if (!tempBlock)
    {
        kernelPanic("Failed to allocate temp block");
    }

    if (readBlock(SUPERBLOCK_BLOCK, tempBlock) != SUCCESS)
    {
        kernelPanic("Failed to read superblock");
    }

    memcpy(sb, tempBlock->block, sizeof(SuperBlock));

    kfree(tempBlock);
    
    if (sb->magicNumber != MAGIC_NUMBER)
    {
        printLine("Failed to read superblock from disk. Creating new one...", YELLOW);
        createSuperBlock();
    }
    else
    {
        printLine("Loading SuperBlock with out creating...", BLUE);
    }
    if (sb->magicNumber != MAGIC_NUMBER)
    {
        kfree(sb);
        kernelPanic("Failed to create superblock!");
    } 
    //print superblock info
    printW("Magic Number: 0x");
    printHexW(sb->magicNumber);
    printLineW("");

    printW("Inode Table Start: ");
    printNumberW(sb->inodeTableStartBlock);
    printLineW("");
    
    printW("Inode Table Blocks: ");
    printNumberW(sb->inodeTableBlocks);
    printLineW("");
    
    printW("Free Blocks: ");
    printNumberW(sb->freeBlocksCount);
    printLineW("");
    
    printW("Free bytes: 0x");
    printHexW(sb->freeBlocksCount * BLOCK_SIZE);
    printW(" bytes");
    printLineW("");

    printLine("SuperBlock loaded successfully!", GREEN);
}

void initializeINodeMap()
{
    INodeTable* inodeTable = (INodeTable*)kmalloc(sizeof(INodeTable));
    if (inodeTable == NULL)
    {
        kernelPanic("Failed to allocate inode table!");
    }
    memset(inodeTable, 0, sizeof(INodeTable));
    Block* block = (Block*)kmalloc(sizeof(Block));
    for (unsigned int i = 0; i < sb->inodeTableBlocks; i++)
    {
        memset(block, 0, sizeof(Block));
        memcpy(block->block, (char*)inodeTable + (i * BLOCK_SIZE), BLOCK_SIZE);
        if (writeBlock(sb->inodeTableStartBlock + i, block) != SUCCESS)
        {
            kfree(inodeTable);
            kfree(block);
            kernelPanic("Failed to write inode table block!");
        }
    }
    kfree(inodeTable);
    kfree(block);
}

void initializeBitmap(unsigned char* bitmap)
{
    Block* block = (Block*)kmalloc(sizeof(Block));
    for (int i = 0; i < TOTAL_BLOCKS / 8; i++)
    {
        bitmap[i] = 0x00;
    }
    for (unsigned int i = 0; i <= SUPERBLOCK_BLOCK; i++)
    {
        bitmap[i / 8] |= (1 << (i % 8));
    }

    for (unsigned int i = sb->inodeTableStartBlock; i < sb->inodeTableStartBlock + sb->inodeTableBlocks; i++)
    {
        bitmap[i / 8] |= (1 << (i % 8));
    }

    for (unsigned int i = sb->bitmapStartBlock; i < sb->bitmapStartBlock + sb->bitmapBlocks; i++)
    {
        bitmap[i / 8] |= (1 << (i % 8));
    }
    for (unsigned int i = 0; i < sb->bitmapBlocks; i++)
    {
        memset(block, 0, sizeof(Block));
        memcpy(block->block, bitmap + (i * BLOCK_SIZE), BLOCK_SIZE);
        if (writeBlock(sb->bitmapStartBlock + i, block) == ERROR) 
        {
            kernelPanic("Failed to write bitmap block!");
        }

    }
}

int allocateBlock()
{
    unsigned char* bitmap = (unsigned char*)kmalloc(sb->bitmapBlocks * BLOCK_SIZE);
    if (bitmap == NULL)
    {
        return ERROR;
    }

    Block* block = (Block*)kmalloc(sizeof(Block));
    if (block == NULL)
    {
        kfree(bitmap);
        return ERROR;
    }

    for (unsigned int i = 0; i < sb->bitmapBlocks; i++)
    {
        if (readBlock(sb->bitmapStartBlock + i, block) != SUCCESS)
        {
            kfree(bitmap);
            kfree(block);
            return ERROR;
        }
        memcpy(bitmap + (i * BLOCK_SIZE), block->block, BLOCK_SIZE);
    }

    int foundBlock = ERROR;
    for (unsigned int i = 0; i < TOTAL_BLOCKS; i++)
    {
        if (!(bitmap[i / 8] & (1 << (i % 8))))
        {
            foundBlock = i;
            break;
        }
    }

    if (foundBlock == ERROR)
    {
        kfree(bitmap);
        kfree(block);
        return ERROR; //no free blocks
    }

    bitmap[foundBlock / 8] |= (1 << (foundBlock % 8));
    sb->freeBlocksCount--;

    //write the modified bitmap block back
    unsigned int bitmapBlockIndex = foundBlock / (BLOCK_SIZE * 8);
    memset(block, 0, sizeof(Block));
    memcpy(block->block, bitmap + (bitmapBlockIndex * BLOCK_SIZE), BLOCK_SIZE);
    if (writeBlock(sb->bitmapStartBlock + bitmapBlockIndex, block) != SUCCESS)
    {
        kfree(bitmap);
        kfree(block);
        return ERROR;
    }

    kfree(bitmap);
    kfree(block);
    return foundBlock;
}

int freeBlock(const unsigned int blockNum)
{
    unsigned int bitmapBlockIndex = blockNum / (BLOCK_SIZE * 8);

    unsigned char* bitmap = (unsigned char*)kmalloc(BLOCK_SIZE);
    if (bitmap == NULL) return ERROR;

    Block* block = (Block*)kmalloc(sizeof(Block));
    if (block == NULL)
    {
        kfree(bitmap);
        return ERROR;
    }

    if (readBlock(sb->bitmapStartBlock + bitmapBlockIndex, block) != SUCCESS)
    {
        kfree(bitmap);
        kfree(block);
        return ERROR;
    }
    memcpy(bitmap, block->block, BLOCK_SIZE);

    unsigned int bitIndex = blockNum % (BLOCK_SIZE * 8);
    bitmap[bitIndex / 8] &= ~(1 << (bitIndex % 8));
    sb->freeBlocksCount++;

    //write back
    memset(block, 0, sizeof(Block));
    memcpy(block->block, bitmap, BLOCK_SIZE);
    if(writeBlock(sb->bitmapStartBlock + bitmapBlockIndex, block) != SUCCESS)
    {
        kfree(bitmap);
        kfree(block);
        return ERROR;
    }

    kfree(bitmap);
    kfree(block);
    return SUCCESS; 
}