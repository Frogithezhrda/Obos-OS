#include "ata.h"


void resetATA()
{
    outb(ATA_DRIVE, 0xE0);          //select master drive
    outb(ATA_CMD, 0x08);            //device reset command
    while (inb(ATA_STATUS) & 0x80); //wait for BSY to clear
    //read status 4 times to flush (ATA spec requires this)
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);
}

int waitForBSY()
{
    int timeout = 1000000;
    while (timeout--)
    {
        unsigned char status = inb(ATA_STATUS);
        if (status & 0x01) return ERROR;
        if (status & 0x20) return ERROR;
        if (!(status & 0x80)) return SUCCESS; 
    }
    return ERROR;
}
int waitForDRQ()
{
    while (inb(ATA_STATUS) & 0x80);

    unsigned char status = 0;
    int timeout = 1000000;
    while (timeout--)
    {
        status = inb(ATA_STATUS);

        if (status & 0x01) return ERROR;
        if (status & 0x20) return ERROR;
        if (status & 0x08) return SUCCESS;
    }
    return SUCCESS;
}

int readATABlock(const int id, Block* block)
{
    unsigned int lba = id * SECTORS_PER_BLOCK;
    char* buffer = block->block;

    disableInterrupts();
    settingDrive(lba);
    outb(ATA_CMD, ATA_CMD_READ_SECTORS);

    for (int s = 0; s < SECTORS_PER_BLOCK; s++)
    {
        if (waitForDRQ() != SUCCESS)
        {
            resetATA();
            enableInterrupts();
            return ERROR;
        }
        for (int i = 0; i < 256; i++)
        {
            unsigned short data = inw(ATA_DATA);
            buffer[(s * 512) + i*2]     = data & 0xFF;
            buffer[(s * 512) + i*2 + 1] = (data >> 8) & 0xFF;
        }
    }

    enableInterrupts();
    return SUCCESS;
}

int writeATABlock(const int id, Block* block)
{
    unsigned int lba = id * SECTORS_PER_BLOCK;
    char* buffer = block->block;

    disableInterrupts();
    settingDrive(lba);
    outb(ATA_CMD, ATA_CMD_WRITE_SECTORS);

    for (int s = 0; s < SECTORS_PER_BLOCK; s++)
    {
        if (waitForDRQ() != SUCCESS)
        {
            resetATA();
            enableInterrupts();
            return ERROR;
        }
        for (int i = 0; i < 256; i++)  //256 words = 512 bytes per sector
        {
            unsigned short data = (unsigned char)buffer[(s * 512) + i*2] 
                                | ((unsigned char)buffer[(s * 512) + i*2 + 1] << 8);
            outw(ATA_DATA, data);
        }
    }

    outb(ATA_CMD, 0xE7);
    if (waitForBSY() != SUCCESS)
    {
        resetATA();
        enableInterrupts();
        return ERROR;
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