#include "PIC.h"

void outb(const unsigned short port, const unsigned char value)
{
    asm volatile ("outb %0, %1" :: "a"(value), "Nd"(port));
}

unsigned char inb(const unsigned short port)
{
    unsigned char value;
    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void maskAllInterrupts(void)
{
    //this just stops all the hardware! interrutpts
    outb(0x21, 0xFF);  // Master PIC
    outb(0xA1, 0xFF);  // Slave PIC
}

void initializePIC()
{
    //only enabling keyboard and timer interrupts for now
    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);
}

void unmaskAllInterrupts()
{
    outb(0x21, 0x00);  // Master PIC
    outb(0xA1, 0x00);  // Slave PIC
}