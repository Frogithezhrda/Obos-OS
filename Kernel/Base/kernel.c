#include "kernel.h"

void obos_main()
{
        disableInterrupts();
        setupIDT();
        initalizeException();
        loadIDT();
        //enableInterrupts();
        clearScreen();
        print("OBOS");

        while(1);
}