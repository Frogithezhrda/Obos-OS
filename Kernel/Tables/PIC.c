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
    outb(MASTER, ALL_MASKED);  // Master PIC
    outb(SLAVE, ALL_MASKED);  // Slave PIC
}

void initializePIC()
{
    //only enabling keyboard and timer interrupts for now
    outb(MASTER, KEYBOARD_TIMER_MASK);
    outb(SLAVE, ALL_MASKED);
}

void unmaskAllInterrupts()
{
    outb(MASTER, NONE_MASKED);  // Master PIC
    outb(SLAVE, NONE_MASKED);  // Slave PIC
}