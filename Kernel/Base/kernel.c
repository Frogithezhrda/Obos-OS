#include "kernel.h"

void obos_main()
{
        disableInterrupts();
        setupIDT();
        initalizeException();
        maskAllInterrupts();
        loadIDT();
        enableInterrupts();
        clearScreen();
        print("OBOS");
        int i = 0;
        float ravos = 1 / i;
        while(1);
}