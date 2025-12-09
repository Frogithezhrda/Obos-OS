#include "kernel.h"

void obos_main()
{
        setupIDT();
        clearScreen();
        print("Ron is the king");
        while(1);
}