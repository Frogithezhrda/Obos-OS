#include "kernel.h"



/*
Few Things to do that in general i didnt do yet that i want to do
Mark Warnings with
Warning: <message> - LIGHT_BROWN color
Error: <message> - red color
Info: <message> - light blue color
Those are for messages


*/




void obos_main()
{
        //basic initalization
        disableInterrupts();
        setupIDT();
        initalizeException();
        initializePIC();
        loadIDT();      
        initializeTimer();
        // maskAllInterrupts(); //no need to maksk interrupts here
        enableInterrupts();
        clearScreen();

        
        initializeMemoryManager();
        printMemoryManagerInfo();


        printNumber(0, WHITE);
        //tests
        printLine("OBOS", WHITE);
        printLine("OBOS Works with \\n", WHITE);
        printChar('A', WHITE);
        printChar('B', WHITE);
        printChar('C', WHITE);
        printNumber(1242, WHITE);
        printCurrentTime();
        int i = 0;
        //float ravos = 1 / i;
        while(1); 
}