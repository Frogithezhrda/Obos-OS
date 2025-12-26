#ifndef PIC_H
#define PIC_H

void outb(const unsigned short port, const unsigned char value);
unsigned char inb(const unsigned short port);
void maskAllInterrupts();
void initializePIC();
void unmaskAllInterrupts();

#endif