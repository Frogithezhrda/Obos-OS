#ifndef PIC_H
#define PIC_H

#define MASTER_PIC_COMMAND_PORT 0x20
#define MASTER_PIC_DATA_PORT 0x21

#define SLAVE_PIC_COMMAND_PORT 0xA0
#define SLAVE_PIC_DATA_PORT 0xA1

#define NONE_MASKED 0x00
#define ALL_MASKED 0xFF
#define KEYBOARD_TIMER_MASK 0xFC

#define EOI 0x20 //EOI - end of interrupt
#define ICW1 0x11
#define ICW2_MASTER_OFFSET 0x20
#define ICW2_SLAVE_OFFSET 0x28
#define ICW3_MASTER_CASCADE 0x04
#define ICW3_SLAVE_ID 0x02
#define ICW4_8086 0x01

#define LOWER_IRQ 8

void outb(const unsigned short port, const unsigned char value);
unsigned char inb(const unsigned short port);
void maskAllInterrupts();
void initializePIC();
void unmaskAllInterrupts();
void endOfInterrupt(const unsigned char irq);

#endif