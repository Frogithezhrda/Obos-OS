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
    outb(MASTER_PIC_DATA_PORT, ALL_MASKED);  // Master PIC
    outb(SLAVE_PIC_DATA_PORT, ALL_MASKED);  // Slave PIC
}

void initializePIC()
{
    //ICW1 - initialize
    outb(MASTER_PIC_COMMAND_PORT, ICW1);
    outb(SLAVE_PIC_COMMAND_PORT, ICW1);
    //ICW2 - remap offset address of IDT
    outb(MASTER_PIC_DATA_PORT, ICW2_MASTER_OFFSET);
    outb(SLAVE_PIC_DATA_PORT, ICW2_SLAVE_OFFSET);
    //ICW3 - setup cascading
    outb(MASTER_PIC_DATA_PORT, ICW3_MASTER_CASCADE);
    outb(SLAVE_PIC_DATA_PORT, ICW3_SLAVE_ID);
    //ICW4 - environment info use mode 8086
    outb(MASTER_PIC_DATA_PORT, ICW4_8086);
    outb(SLAVE_PIC_DATA_PORT, ICW4_8086);
    //only enabling keyboard and timer interrupts for now
    outb(MASTER_PIC_DATA_PORT, KEYBOARD_TIMER_MASK);
    outb(SLAVE_PIC_DATA_PORT, ALL_MASKED);
}

void unmaskAllInterrupts()
{
    outb(MASTER_PIC_DATA_PORT, NONE_MASKED);  // Master PIC
    outb(SLAVE_PIC_DATA_PORT, NONE_MASKED);  // Slave PIC
}

void endOfInterrupt(const unsigned char irq)
{
    if (irq >= LOWER_IRQ) 
    {
        outb(SLAVE_PIC_COMMAND_PORT, EOI);
    }
    outb(MASTER_PIC_COMMAND_PORT, EOI);
}