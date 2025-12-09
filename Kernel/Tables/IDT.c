#include "IDT.h"

static IDTEntry idt[IDT_SIZE];
static IDTPointer idtPointer;

void setupIDT(void)
{
    idtPointer.limit = sizeof(IDTEntry) * IDT_SIZE - 1;
    memset(idt, 0, sizeof(IDTEntry) * IDT_SIZE);
    idtPointer.offset = (int)&idt;
    loadIDT();
}

void loadIDT(void)
{
    asm volatile ("lidt %0" : : "m"(idtPointer));
}

void createIDTGate(const char index, const int address, const short select, const char attr)
{

  idt[index].typeAttr = attr;
  idt[index].offsetLow = (short)(address & 0xFFFF); 
  idt[index].offsetHigh = (short)(address >> 16);
  idt[index].selector = select;
  idt[index].zero = 0;
}