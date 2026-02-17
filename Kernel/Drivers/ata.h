#ifndef ATA_H
#define ATA_H

#include "../Tables/IDT.h"
#include "../Drivers/consoleDriver.h"

#define BLOCK_SIZE 512
#define SECTORS_PER_BLOCK 1
#define ERROR -1
#define SUCCESS 0

#define ATA_DATA        0x1F0
#define ATA_ERROR       0x1F1
#define ATA_SECTOR_CNT  0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE       0x1F6
#define ATA_CMD         0x1F7
#define ATA_STATUS      0x1F7

#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_WRITE_SECTORS 0x30

typedef struct Block
{
    char block[BLOCK_SIZE];
} Block;


int readATABlock(const int id, Block* block);
int writeATABlock(const int id, Block* block);
int waitForDRQ();
void settingDrive(const int lba);



#endif