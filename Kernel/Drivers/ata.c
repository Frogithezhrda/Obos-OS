#include "ata.h"

int waitForDRQ()
{
    while (inb(ATA_STATUS) & 0x80);

    unsigned char status = inb(ATA_STATUS);
    if (!(status & 0x08)) return ERROR;
    return SUCCESS;
}

int readATABlock(const int id, Block* block)
{
    unsigned int lba = id * SECTORS_PER_BLOCK;
    char* buffer = block->block;
    disableInterrupts();
    //reading a sector
    settingDrive(lba);

    outb(ATA_CMD, ATA_CMD_READ_SECTORS);
    //waiting
    if (waitForDRQ() != SUCCESS)
    {
        enableInterrupts();
        return ERROR;
    } 
    for (int i = 0; i < BLOCK_SIZE / 2; i++)
    {
        unsigned short data = inw(ATA_DATA);
        buffer[i*2] = data & 0xFF;
        buffer[i*2+1] = (data >> 8) & 0xFF;
    }
    block->block[BLOCK_SIZE - 1] = '\0';
    enableInterrupts();
    return SUCCESS;
}

int writeATABlock(const int id, Block* block)
{
    unsigned int lba = id * SECTORS_PER_BLOCK;
    char* buffer = block->block;
    disableInterrupts();
    //reading a sector
    settingDrive(lba);
    outb(ATA_CMD, ATA_CMD_WRITE_SECTORS);
    //waiting
    if (waitForDRQ() != SUCCESS)
    {
        enableInterrupts();
        return ERROR;
    } 

    for (int i = 0; i < BLOCK_SIZE / 2; i++)
    {
        unsigned short data = buffer[i*2] + (buffer[i*2+1] << 8);
        outw(ATA_DATA, data);
    }

    enableInterrupts();
    return SUCCESS;
}

void settingDrive(const int lba)
{
    outb(ATA_SECTOR_CNT, 1); //reading 1 sector
    outb(ATA_LBA_LOW,  lba & 0xFF);
    outb(ATA_LBA_MID,  (lba >> 8) & 0xFF);
    outb(ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F)); //selecting master upper 4 bits LBA
}