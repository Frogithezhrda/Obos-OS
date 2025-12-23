#ifndef IDT_H
#define IDT_H

#include "../SystemLib/obosMemory.h"

#define IDT_SIZE 256

enum {
    GATE = 0x8E
};
typedef struct IDTEntry 
{
    short offsetLow;    // first 16 bit offset
    short selector;     // code segment in GDT
    char  zero;         // zero its a pad always 0
    char  typeAttr;     // type and attributes(gate)
    short offsetHigh;   // last 16 bits
} IDTEntry;

typedef struct IDTPointer {

    short limit;
    int offset;
} IDTPointer;

void setupIDT(void);
void loadIDT(void);
void createIDTGate(const char index, const int address, const short select, const char attr);
void registerInterruptHandler(const char interruptNumber, void (*handler)(), const short selector, const char typeAttr);
void disableInterrupts();
void enableInterrupts();

#endif