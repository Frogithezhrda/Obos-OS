#include "IDT.h"

extern void* ExceptionHandlers[41];
extern void pageFaultISR();

static IDTEntry idt[IDT_SIZE];
static IDTPointer idtPointer;

void setupIDT(void)
{
    idtPointer.limit = sizeof(IDTEntry) * IDT_SIZE - 1;
    memset(idt, 0, sizeof(IDTEntry) * IDT_SIZE);
    idtPointer.offset = (unsigned int)&idt;
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

void initalizeException()
{
    //registering first 32 exception handlers
    for(char interruptNumber = 0; interruptNumber < CPU_EXCEPTION_COUNT; interruptNumber++)
    {
        if(interruptNumber == 14) continue;
        registerInterruptHandler(interruptNumber, ExceptionHandlers[interruptNumber], CODE_SEGMENT, GATE);
    }
    //registering hardware interrupts currently only timer and keyboard
    registerInterruptHandler(TIMER_INTERRUPT_VECTOR, ExceptionHandlers[TIMER_INTERRUPT_VECTOR], CODE_SEGMENT, GATE);
    registerInterruptHandler(KEYBOARD_INTERRUPT_VECTOR, ExceptionHandlers[KEYBOARD_INTERRUPT_VECTOR], CODE_SEGMENT, GATE);
    registerInterruptHandler(RTC_INTERRUPT_VECTOR, ExceptionHandlers[RTC_INTERRUPT_VECTOR], CODE_SEGMENT, GATE); //RTC interrupt vector is 40
    registerInterruptHandler(14, pageFaultISR, CODE_SEGMENT, GATE);
    if (idt[0].offsetLow == 0 && idt[0].offsetHigh == 0) 
    {
        print("ERROR: IDT entry 0 is empty!", RED);
        while(1);
    }
}