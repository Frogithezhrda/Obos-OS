#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include "blockdevice.h"
#include "../SystemLib/obosMemory.h"

#define TOTAL_BLOCKS 1024
#define SUPERBLOCK_BLOCK 512
#define MAGIC_NUMBER 0x4F424653
#define MAX_BLOCKS_PER_FILE 16
#define MAX_FILES 128
#define FILE_NAME_LENGTH 64
#define DIR_ENTRY_SIZE (sizeof(DirectoryEntry))


typedef enum Type {File =0, Directory} Type;

typedef struct INode
{
    Type type;
    unsigned int fileSize;
    unsigned int blocks[MAX_BLOCKS_PER_FILE];
    char isUsed;
} INode;


typedef struct INodeTable
{
    INode inodes[MAX_FILES];
} INodeTable;

typedef struct DirectoryEntry
{
    unsigned int inodeNumber;
    char name[FILE_NAME_LENGTH];
} DirectoryEntry;

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
void initializeBitmap(unsigned char* newBitmap);
void initializeINodeMap();
void createRootDirectory();
void readFile(const char* name, char* buffer, unsigned int size);
void writeFile(const char* name, const char* data, unsigned int size);
/*
if it returns error you do not let him read/write to the file
and quit the file operation immediatly
*/
int allocateBlock();
/*
same here as allocation you must stop the user program if it fails

*/
int freeBlock(const unsigned int blockNum);

int createFile(const char* name, Type type);
void ls();
#define createFileF(name) createFile(name, File)

void writeFile(const char* name, const char* data, unsigned int size);
void readFile(const char* name, char* buffer, unsigned int size);


#endif