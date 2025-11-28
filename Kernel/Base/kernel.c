#include "kernel.h"

void obos_main()
{
        char* videoMem = (char*)0xB8000;
        videoMem[0] = 'R';
        videoMem[1] = 0x0F;
        while(1);
}