#include "superblock.h"

SuperBlock* sb = NULL;
INodeTable* inodeTable = NULL;
unsigned char* bitmap = NULL;
unsigned int currentDirINode = 0;

static int findFile(const char* name)
{
    INode* dir = &inodeTable->inodes[currentDirINode];
    unsigned int totalEntries = dir->fileSize / DIR_ENTRY_SIZE;
    unsigned int entriesRead = 0;

    Block* block = (Block*)kmalloc(sizeof(Block));
    if (!block) return ERROR;

    for (unsigned int b = 0; b < MAX_BLOCKS_PER_FILE && entriesRead < totalEntries; b++)
    {
        if (dir->blocks[b] == 0) break;

        memset(block, 0, sizeof(Block));
        if (readBlock(dir->blocks[b], block) != SUCCESS)
        {
            kfree(block);
            return ERROR;
        }

        unsigned int entriesPerBlock = BLOCK_SIZE / DIR_ENTRY_SIZE;
        for (unsigned int e = 0; e < entriesPerBlock && entriesRead < totalEntries; e++, entriesRead++)
        {
            DirectoryEntry* entry = (DirectoryEntry*)(block->block + (e * DIR_ENTRY_SIZE));
            if (strcmp(entry->name, name) == 0)
            {
                unsigned int inodeIdx = entry->inodeNumber;
                kfree(block);
                return (int)inodeIdx;
            }
        }
    }

    kfree(block);
    return ERROR;
}


static void flushInodeTable()
{
    Block* block = (Block*)kmalloc(sizeof(Block));
    if (!block) kernelPanic("Failed to allocate block buffer!");
    for (unsigned int i = 0; i < sb->inodeTableBlocks; i++)
    {
        memset(block, 0, sizeof(Block));
        memcpy(block->block, (char*)inodeTable + (i * BLOCK_SIZE), BLOCK_SIZE);
        if (writeBlock(sb->inodeTableStartBlock + i, block) != SUCCESS)
        {
            kfree(block);
            kernelPanic("Failed to flush inode table!");
        }
    }
    kfree(block);
}


static int addDirEntry(unsigned int fileInodeIndex, const char* name)
{
    INode* dir = &inodeTable->inodes[currentDirINode];

    //find which block + slot has room, or allocate a new block
    unsigned int entriesPerBlock = BLOCK_SIZE / DIR_ENTRY_SIZE;
    unsigned int totalEntries = dir->fileSize / DIR_ENTRY_SIZE;
    unsigned int blockIndex = totalEntries / entriesPerBlock;
    unsigned int slotIndex  = totalEntries % entriesPerBlock;

    Block* block = (Block*)kmalloc(sizeof(Block));
    if (!block) return ERROR;

    if (slotIndex == 0)
    {
        if (blockIndex >= MAX_BLOCKS_PER_FILE)
        {
            kfree(block);
            return ERROR; //dir full
        }
        int newBlock = allocateBlock();
        if (newBlock == ERROR)
        {
            kfree(block);
            return ERROR;
        }
        dir->blocks[blockIndex] = newBlock;
        memset(block, 0, sizeof(Block));
    }
    else
    {
        //load block
        if (readBlock(dir->blocks[blockIndex], block) != SUCCESS)
        {
            kfree(block);
            return ERROR;
        }
    }

    //write the entry into the slot
    unsigned char* ptr = block->block + (slotIndex * DIR_ENTRY_SIZE);
    *((unsigned int*)ptr) = fileInodeIndex;
    memset(ptr + 4, 0, 64);
    // opy name max 63 chars + null terminator
    unsigned int nameLen = 0;
    while (name[nameLen] && nameLen < FILE_NAME_LENGTH - 1) nameLen++;
    memcpy(ptr + 4, name, nameLen);

    if (writeBlock(dir->blocks[blockIndex], block) != SUCCESS)
    {
        kfree(block);
        return ERROR;
    }

    dir->fileSize += DIR_ENTRY_SIZE;
    flushInodeTable();

    kfree(block);
    return SUCCESS;
}

static int removeDirEntry(const char* name)
{
    INode* dir = &inodeTable->inodes[currentDirINode];
    unsigned int totalEntries = dir->fileSize / DIR_ENTRY_SIZE;
    unsigned int entriesPerBlock = BLOCK_SIZE / DIR_ENTRY_SIZE;
    unsigned int entriesRead = 0;

    Block* block = (Block*)kmalloc(sizeof(Block));
    if (!block) return ERROR;

    for (unsigned int b = 0; b < MAX_BLOCKS_PER_FILE && entriesRead < totalEntries; b++)
    {
        if (dir->blocks[b] == 0) break;

        memset(block, 0, sizeof(Block));
        if (readBlock(dir->blocks[b], block) != SUCCESS)
        {
            kfree(block);
            return ERROR;
        }

        for (unsigned int e = 0; e < entriesPerBlock && entriesRead < totalEntries; e++, entriesRead++)
        {
            DirectoryEntry* entry = (DirectoryEntry*)(block->block + (e * DIR_ENTRY_SIZE));
            if (strcmp(entry->name, name) == 0)
            {
                // replace this entry with the last entry to fill the gap
                unsigned int lastBlock  = (totalEntries - 1) / entriesPerBlock;
                unsigned int lastSlot   = (totalEntries - 1) % entriesPerBlock;

                if (b == lastBlock && e == lastSlot)
                {
                    // it IS the last entry — just zero it
                    memset(entry, 0, DIR_ENTRY_SIZE);
                }
                else
                {
                    // load the last block and copy last entry here
                    Block* lastBlockBuf = (Block*)kmalloc(sizeof(Block));
                    if (!lastBlockBuf) { kfree(block); return ERROR; }

                    memset(lastBlockBuf, 0, sizeof(Block));
                    if (readBlock(dir->blocks[lastBlock], lastBlockBuf) != SUCCESS)
                    {
                        kfree(lastBlockBuf);
                        kfree(block);
                        return ERROR;
                    }

                    DirectoryEntry* last = (DirectoryEntry*)(lastBlockBuf->block + (lastSlot * DIR_ENTRY_SIZE));
                    memcpy(entry, last, DIR_ENTRY_SIZE);
                    memset(last, 0, DIR_ENTRY_SIZE);

                    if (writeBlock(dir->blocks[lastBlock], lastBlockBuf) != SUCCESS)
                    {
                        kfree(lastBlockBuf);
                        kfree(block);
                        return ERROR;
                    }
                    kfree(lastBlockBuf);
                }

                // write the modified block back
                if (writeBlock(dir->blocks[b], block) != SUCCESS)
                {
                    kfree(block);
                    return ERROR;
                }

                dir->fileSize -= DIR_ENTRY_SIZE;
                flushInodeTable();
                kfree(block);
                return SUCCESS;
            }
        }
    }
    return ERROR;
}


static void loadInodeTable()
{
    if (inodeTable == NULL) inodeTable = (INodeTable*)kmalloc(sizeof(INodeTable));
    if (inodeTable == NULL) kernelPanic("Failed to allocate inode table!");

    Block* block = (Block*)kmalloc(sizeof(Block));
    if (!block) kernelPanic("Failed to allocate block buffer!");
    for (unsigned int i = 0; i < sb->inodeTableBlocks; i++)
    {
        memset(block, 0, sizeof(Block));
        if (readBlock(sb->inodeTableStartBlock + i, block) != SUCCESS)
        {
            kfree(block);
            kernelPanic("Failed to load inode table!");
        }
        memcpy((char*)inodeTable + (i * BLOCK_SIZE), block->block, BLOCK_SIZE);
    }
    kfree(block);
}

static void flushBitmap()
{
    Block* block = (Block*)kmalloc(sizeof(Block));
    if (!block) kernelPanic("Failed to allocate block buffer!");
    for (unsigned int i = 0; i < sb->bitmapBlocks; i++)
    {
        memset(block, 0, sizeof(Block));
        memcpy(block->block, bitmap + (i * BLOCK_SIZE), BLOCK_SIZE);
        if (writeBlock(sb->bitmapStartBlock + i, block) != SUCCESS)
        {
            kfree(block);
            kernelPanic("Failed to flush bitmap!");
        }
    }
    kfree(block);
}

static void loadBitmap()
{
    if (bitmap == NULL) bitmap = (unsigned char*)kmalloc(sb->bitmapBlocks * BLOCK_SIZE);
    if (bitmap == NULL) kernelPanic("Failed to allocate bitmap!");

    Block* block = (Block*)kmalloc(sizeof(Block));
    if (!block) kernelPanic("Failed to allocate block buffer!");
    for (unsigned int i = 0; i < sb->bitmapBlocks; i++)
    {
        memset(block, 0, sizeof(Block));
        if (readBlock(sb->bitmapStartBlock + i, block) != SUCCESS)
        {
            kfree(block);
            kernelPanic("Failed to load bitmap!");
        }
        memcpy(bitmap + (i * BLOCK_SIZE), block->block, BLOCK_SIZE);
    }
    kfree(block);
}


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
    createRootDirectory();
    printLine("Root directory created successfully!", GREEN);
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
    loadInodeTable();
    loadBitmap();
    printLine("SuperBlock loaded successfully!", GREEN);
}

void initializeINodeMap()
{
    if (inodeTable == NULL) inodeTable = (INodeTable*)kmalloc(sizeof(INodeTable));
    if (inodeTable == NULL) kernelPanic("Failed to allocate inode table!");
    memset(inodeTable, 0, sizeof(INodeTable));
    flushInodeTable();
}

void initializeBitmap(unsigned char* newBitmap)
{
    if (bitmap != NULL) kfree(bitmap);
    bitmap = newBitmap;

    memset(bitmap, 0x00, sb->bitmapBlocks * BLOCK_SIZE);

    // Mark superblock block(s) as used
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

    flushBitmap();
}

int allocateBlock()
{
    int foundBlock = ERROR;
    for (unsigned int i = 0; i < TOTAL_BLOCKS; i++)
    {
        if (!(bitmap[i / 8] & (1 << (i % 8))))
        {
            foundBlock = i;
            break;
        }
    }
    if (foundBlock == ERROR) return ERROR;

    bitmap[foundBlock / 8] |= (1 << (foundBlock % 8));
    sb->freeBlocksCount--;

    unsigned int bitmapBlockIndex = foundBlock / (BLOCK_SIZE * 8);
    Block* block = (Block*)kmalloc(sizeof(Block));
    if (!block) return ERROR;

    memset(block, 0, sizeof(Block));
    memcpy(block->block, bitmap + (bitmapBlockIndex * BLOCK_SIZE), BLOCK_SIZE);

    if (writeBlock(sb->bitmapStartBlock + bitmapBlockIndex, block) != SUCCESS)
    {
        kfree(block);
        return ERROR;
    }
    kfree(block);
    return foundBlock;
}

int freeBlock(const unsigned int blockNum)
{
    unsigned int bitmapBlockIndex = blockNum / (BLOCK_SIZE * 8);
    unsigned int bitIndex = blockNum % (BLOCK_SIZE * 8);

    bitmap[bitIndex / 8] &= ~(1 << (bitIndex % 8));
    sb->freeBlocksCount++;

    Block* block = (Block*)kmalloc(sizeof(Block));
    if (!block) return ERROR;
    memset(block, 0, sizeof(Block));
    memcpy(block->block, bitmap + (bitmapBlockIndex * BLOCK_SIZE), BLOCK_SIZE);
    if (writeBlock(sb->bitmapStartBlock + bitmapBlockIndex, block) != SUCCESS)
    {
        kfree(block);
        return ERROR;
    }
    kfree(block);
    return SUCCESS;
}

int createFile(const char* name, Type type)
{
    int freeInodeIndex = ERROR;
    for (unsigned int i = 0; i < MAX_FILES; i++)
    {
        if (!inodeTable->inodes[i].isUsed)
        {
            freeInodeIndex = i;
            break;
        }
    }
    if (freeInodeIndex == ERROR) return ERROR;
    inodeTable->inodes[freeInodeIndex].isUsed = 1;
    inodeTable->inodes[freeInodeIndex].type = type;
    inodeTable->inodes[freeInodeIndex].fileSize = 0;
    
    flushInodeTable();
    addDirEntry(freeInodeIndex, name);
    return freeInodeIndex;
}

void createRootDirectory()
{
    int rootInodeIndex = createFile("root", Directory);
    if (rootInodeIndex == ERROR) kernelPanic("Failed to create root directory!");
    printLine("Created root directory with inode index: ", LIGHT_BLUE);
    printNumber(rootInodeIndex, LIGHT_BLUE);

    int dataBlock = allocateBlock();
    if (dataBlock == ERROR) kernelPanic("Failed to allocate block for root directory!");

    inodeTable->inodes[rootInodeIndex].fileSize = DIR_ENTRY_SIZE * 2;
    inodeTable->inodes[rootInodeIndex].blocks[0] = dataBlock;
    flushInodeTable();

    //write "." and ".." entries into the data block
    Block* block = (Block*)kmalloc(sizeof(Block));
    if (!block) kernelPanic("Failed to allocate block buffer!");
    memset(block, 0, sizeof(Block));

    unsigned char* ptr = block->block;
    *((unsigned int*)(ptr)) = (unsigned int)rootInodeIndex;
    memcpy((char*)(ptr + 4), ".", 2);

    *((unsigned int*)(ptr + DIR_ENTRY_SIZE)) = (unsigned int)rootInodeIndex;
    memcpy((char*)(ptr + DIR_ENTRY_SIZE + 4), "..", 3);

    if (writeBlock(dataBlock, block) != SUCCESS)
    {
        kfree(block);
        kernelPanic("Failed to write root directory block!");
    }
    kfree(block);
}


void ls()
{
    INode* dir = &inodeTable->inodes[currentDirINode];

    if (!dir->isUsed || dir->type != Directory)
    {
        printLine("Not a valid directory!", RED);
        return;
    }

    unsigned int totalEntries = dir->fileSize / DIR_ENTRY_SIZE;
    unsigned int entriesRead = 0;

    Block* block = (Block*)kmalloc(sizeof(Block));
    if (!block) kernelPanic("Failed to allocate block buffer!");

    for (unsigned int b = 0; b < MAX_BLOCKS_PER_FILE && entriesRead < totalEntries; b++)
    {
        if (dir->blocks[b] == 0) break;

        memset(block, 0, sizeof(Block));
        if (readBlock(dir->blocks[b], block) != SUCCESS)
        {
            printLine("Failed to read directory block!", RED);
            break;
        }

        unsigned int entriesPerBlock = BLOCK_SIZE / DIR_ENTRY_SIZE;
        for (unsigned int e = 0; e < entriesPerBlock && entriesRead < totalEntries; e++, entriesRead++)
        {
            unsigned char* ptr = block->block + (e * DIR_ENTRY_SIZE);
            unsigned int inodeIdx = *((unsigned int*)ptr);
            char* name = (char*)(ptr + 4);

            printW(name);
            printW(inodeTable->inodes[inodeIdx].type == Directory ? "  [DIR]\n" : "  [FILE]\n");
        }
    }
    kfree(block);
}

void readFile(const char* name, char* buffer, unsigned int size)
{
    int inodeIdx = findFile(name);
    if (inodeIdx == ERROR)
    {
        printLine("File not found!", RED);
        return;
    }
    
    INode* inode = &inodeTable->inodes[inodeIdx];
    if (inode->type != File)
    {
        printLine("Not a file!", RED);
        return;
    }

    unsigned int bytesRead = 0;
    Block* block = (Block*)kmalloc(sizeof(Block));
    if (!block) return;

    for (unsigned int b = 0; b < MAX_BLOCKS_PER_FILE && bytesRead < size; b++)
    {
        if (inode->blocks[b] == 0) break;

        memset(block, 0, sizeof(Block));
        if (readBlock(inode->blocks[b], block) != SUCCESS) break;

        unsigned int toCopy = size - bytesRead;
        if (toCopy > BLOCK_SIZE) toCopy = BLOCK_SIZE;
        memcpy(buffer + bytesRead, block->block, toCopy);
        bytesRead += toCopy;
    }
    kfree(block);
}

void writeFile(const char* name, const char* data, unsigned int size)
{
    int inodeIdx = findFile(name);
    if (inodeIdx == ERROR)
    {
        printLine("File not found!", RED);
        return;
    }
    INode* inode = &inodeTable->inodes[inodeIdx];
    if (inode->type != File)
    {
        printLine("Not a file!", RED);
        return;
    }

    unsigned int bytesWritten = 0;
    Block* block = (Block*)kmalloc(sizeof(Block));
    if (!block) return;

    for (unsigned int b = 0; b < MAX_BLOCKS_PER_FILE && bytesWritten < size; b++)
    {
        if (inode->blocks[b] == 0)
        {
            int newBlock = allocateBlock();
            if (newBlock == ERROR) break;
            inode->blocks[b] = newBlock;
        }

        memset(block, 0, sizeof(Block));
        unsigned int toWrite = size - bytesWritten;
        if (toWrite > BLOCK_SIZE) toWrite = BLOCK_SIZE;
        memcpy(block->block, data + bytesWritten, toWrite);

        if (writeBlock(inode->blocks[b], block) != SUCCESS) break;
        bytesWritten += toWrite;
    }

    inode->fileSize = bytesWritten;
    flushInodeTable();
    kfree(block);
}

int deleteFile(const char* name)
{
    int inodeIdx = findFile(name);
    if (inodeIdx == ERROR)
    {
        printLine("File not found!", RED);
        return ERROR;
    }
    if(inodeIdx == 0 || !strcmp(name, ".") || !strcmp(name, ".."))
    {
        printLine("Cannot delete root directory!", RED);
        return ERROR;
    }
    INode* inode = &inodeTable->inodes[inodeIdx];
    //free the blocks associated with the inode
    for (unsigned int b = 0; b < MAX_BLOCKS_PER_FILE; b++)
    {
        if (inode->blocks[b] != 0)
        {
            freeBlock(inode->blocks[b]);
            inode->blocks[b] = 0;
        }
    }

    inode->isUsed = 0;
    removeDirEntry(name);
    flushInodeTable();
    return SUCCESS;
}