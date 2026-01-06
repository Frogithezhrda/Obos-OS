#include "kernel.h"

void obos_main()
{
        disableInterrupts();
        setupIDT();
        initalizeException();
        initializePIC();
        loadIDT();      
        initializeTimer();
        // maskAllInterrupts(); //no need to maksk interrupts here
        enableInterrupts();
        clearScreen();
        printLine("OBOS", WHITE);
        printLine("OBOS Works with \\n", WHITE);
        int i = 0;
        //float ravos = 1 / i;
        while(1); 
}