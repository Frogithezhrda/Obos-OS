#include "kernel.h"

void obos_main()
{
        disableInterrupts();
        setupIDT();
        initalizeException();
        initializePIC();
        loadIDT();
        initializeTimer();
        initializeKeyboard(); 
        // maskAllInterrupts();
        enableInterrupts();
        clearScreen();
        print("OBOS");
        int i = 2;
        float ravos = 1 / i;
        while(1);
}