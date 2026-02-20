#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include "blockdevice.h"
#include "../SystemLib/obosMemory.h"

#define TOTAL_BLOCKS 1024
#define SUPERBLOCK_BLOCK 40
#define MAGIC_NUMBER 0x4F424653
#define MAX_BLOCKS_PER_FILE 16
#define MAX_FILES 128

typedef enum Type {File =0, Directory} Type;

typedef struct INode
{
    Type type;
    unsigned int fileSize;
    unsigned int blocks[MAX_BLOCKS_PER_FILE];
} INode;


typedef struct INodeTable
{
    INode inodes[MAX_FILES];
} INodeTable;


typedef struct SuperBlock
{
    unsigned int magicNumber;
    unsigned int freeBlocksCount;
    unsigned int totalBlocksCount;
    unsigned int inodeTableStartBlock;
    unsigned int inodeTableBlocks;
    unsigned int bitmapStartBlock;
    unsigned int bitmapBlocks;
} SuperBlock;

void createSuperBlock();
void loadSuperBlock();
void initializeBitmap(unsigned char* bitmap);
void initializeINodeMap();

#endif