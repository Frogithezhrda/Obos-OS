#ifndef PIC_H
#define PIC_H

#define MASTER		0x21
#define SLAVE		0xA1
#define NONE_MASKED 0x00
#define ALL_MASKED 0xFF
#define KEYBOARD_TIMER_MASK 0xFC
#de

void outb(const unsigned short port, const unsigned char value);
unsigned char inb(const unsigned short port);
void maskAllInterrupts();
void initializePIC();
void unmaskAllInterrupts();

#endif