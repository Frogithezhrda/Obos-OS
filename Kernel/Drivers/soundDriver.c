#include "soundDriver.h"

static void dspWrite(char cmd)
{
    while (inb(SB16_WRITE) & 0x80);
    outb(SB16_WRITE, cmd);
}
static void dmaSetup(char* buf, unsigned int len) 
{
    unsigned int addr = (unsigned int)buf;
    unsigned int page = (addr >> 16) & 0xFF;
    unsigned short offset = addr & 0xFFFF;

    outb(0x0A, 0x05);               // mask DMA channel 1
    outb(0x0C, 0x00);               // flip-flop reset
    outb(0x0B, 0x49);               // single transfer, read, autoinit off, channel 1
    outb(0x02, offset & 0xFF);      // address low
    outb(0x02, (offset >> 8));      // address high
    outb(0x83, page);               // page register for channel 1
    outb(0x03, (len - 1) & 0xFF);   // count low
    outb(0x03, ((len - 1) >> 8));   // count high
    outb(0x0A, 0x01);               // unmask channel 1
}
static char dspRead(void) 
{
    while (!(inb(SB16_STATUS) & 0x80));
    return inb(SB16_READ);
}

void soundBlasterInit()
{
    outb(SB16_RESET, 0x1);
    for (volatile int i = 0; i < 100000; i++);
    outb(SB16_RESET, 0x0);
}

void playSound(char* samples, unsigned int length, unsigned int rate)
{
    dmaSetup(samples, length);

    // Set sample rate
    dspWrite(0x41);                        // set output rate command
    dspWrite((rate >> 8) & 0xFF);   // high byte
    dspWrite(rate & 0xFF);          // low byte

    // Start 8-bit single-cycle DMA transfer
    dspWrite(0xC6);                        // 8-bit output command
    dspWrite(0x00);                        // mono, unsigned
    dspWrite((length - 1) & 0xFF);            // length low
    dspWrite(((length - 1) >> 8) & 0xFF);     // length high
}

void stopSound(void)
{
    outb(SB16_RESET, 0x1);
    for (volatile int i = 0; i < 100000; i++);
    outb(SB16_RESET, 0x0);
    dspRead();
}