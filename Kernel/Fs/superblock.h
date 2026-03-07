#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include "blockdevice.h"
#include "../SystemLib/obosMemory.h"

#define TOTAL_BLOCKS 1024
#define SUPERBLOCK_BLOCK 200
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
    unsigned int parentINode;
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

/**
 * Creates and initializes a new superblock on disk.
 *
 * Sets magic number, total block count, free block count,
 * inode table location/size, bitmap location/size.
 *
 * Must be called when formatting the filesystem.
 */
void createSuperBlock();

/**
 * Loads the superblock from disk into memory.
 *
 * Validates magic number to ensure filesystem integrity.
 * Should panic or halt if magic number is invalid.
 */
void loadSuperBlock();

/**
 * Initializes the block allocation bitmap.
 *
 * Marks reserved blocks (superblock, inode table, bitmap blocks)
 * as used and all remaining blocks as free.
 *
 * newBitmap must point to a memory region large enough
 * to hold all bitmap blocks.
 */
void initializeBitmap(unsigned char* newBitmap);

/**
 * Initializes all inodes.
 *
 * Sets isUsed = 0 for all entries.
 * Clears block lists and metadata.
 *
 * Called during filesystem formatting.
 */
void initializeINodeMap();

/**
 * Creates the root directory inode.
 *
 * Allocates inode 0 (or first free inode),
 * sets its type to Directory,
 * allocates initial data block if needed,
 * sets parent to itself.
 */
void createRootDirectory();

/**
 * Reads file data into buffer.
 *
 * name  - file name in current directory
 * buffer - destination buffer
 * size  - maximum number of bytes to read
 *
 * Stops immediately if file not found or is not a regular file.
 */
int readFile(const char* name, char* buffer, unsigned int size);

/**
 * Writes data into a file.
 *
 * name - file name in current directory
 * data - source buffer
 * size - number of bytes to write
 *
 * Allocates new blocks if needed.
 * Stops immediately if allocation fails.
 */
void writeFile(const char* name, const char* data, unsigned int size);

/**
 * Reads file data into buffer.
 *
 * name  - file name in current directory
 * buffer - destination buffer
 * size  - maximum number of bytes to read
 *
 * Stops immediately if file not found or is not a regular file.
 */
int allocateBlock();

/**
 * Frees a previously allocated block.
 *
 * blockNum - block number to free
 *
 * Returns:
 *   0  -> success
 *  -1  -> invalid block number or already free
 *
 * Updates bitmap and superblock freeBlocksCount.
 */
int freeBlock(const unsigned int blockNum);

/**
 * Creates a file or directory in the current directory.
 *
 * name - file/directory name
 * type - File or Directory
 *
 * Allocates a new inode and directory entry.
 *
 * Returns:
 *   >= 0  -> inode number
 *   -1    -> failure (no free inode, name exists, etc.)
 */
int createFile(const char* name, Type type);

/**
 * Lists all entries in the current directory.
 *
 * Prints file names and optionally their type.
 */
void ls();

/**
 * Deletes a file from the current directory.
 *
 * Frees all allocated data blocks.
 * Frees inode.
 * Removes directory entry.
 *
 * Returns:
 *   0  -> success
 *  -1  -> file not found or directory not empty
 */
int deleteFile(const char* name);

/**
 * Creates a new directory inside current directory.
 *
 * Equivalent to createFile(name, Directory).
 *
 * Returns:
 *   >= 0 -> inode number
 *   -1   -> failure
 */
int createDir(const char* name);

/**
 * Changes current working directory.
 *
 * name - directory name or ".." for parent
 *
 * Updates currentDirINode.
 *
 * Returns:
 *   0  -> success
 *  -1  -> directory not found or not a directory
 */
int cd(const char* name);

#define createFileF(name) createFile(name, File)

#endif