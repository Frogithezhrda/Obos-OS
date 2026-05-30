#include "gui.h"


unsigned int isGUIInitialized = 0;


void openGUI()
{
    isGUIInitialized = 1;

    gfxInit();
    gfxClear(WHITE);
    gfxFlush();
    while(1);
}