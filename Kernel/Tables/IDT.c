#include "IDT.h"

static IDTEntry idt[IDT_SIZE];
static IDTPointer idtPointer;

void setupIDT(void)
{
    idtPointer.limit = sizeof(IDTEntry) * IDT_SIZE - 1;
    memset(idt, 0, sizeof(IDTEntry) * IDT_SIZE);
    idtPointer.offset = (unsigned int)&idt;
    loadIDT();
}

void loadIDT(void)
{
    asm volatile ("lidt %0" : : "m"(idtPointer));
}

void disableInterrupts()
{
    asm volatile ("cli");
}

void enableInterrupts()
{
    asm volatile ("sti");
}

void registerInterruptHandler(const char interruptNumber, void (*handler)(), const short selector, const char typeAttr)
{
    int address = (int)handler;
    idt[interruptNumber].offsetLow = (short)(address & 0xFFFF);
    idt[interruptNumber].selector  = selector;
    idt[interruptNumber].zero      = 0;
    idt[interruptNumber].typeAttr  = typeAttr;
    idt[interruptNumber].offsetHigh = (short)((address >> 16) & 0xFFFF);
}

void createIDTGate(const char index, const int address, const short select, const char attr)
{
  idt[index].typeAttr = attr;
  idt[index].offsetLow = (short)(address & 0xFFFF); 
  idt[index].offsetHigh = (short)(address >> 16);
  idt[index].selector = select;
  idt[index].zero = 0;
} 