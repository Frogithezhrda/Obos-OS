#include "gui.h"
#include "../Desktop/wm.h"

unsigned int isGUIInitialized = 0;

void openGUI()
{
    isGUIInitialized = 1;
    gfxInit();
    wmAddWindow(100, 100, 300, 200, "Hello", (Color){230, 230, 230});
    wmAddWindow(250, 180, 300, 200, "Second", (Color){255, 240, 200});
    unsigned int last = getTicks();
    while (1)
    {
        unsigned int now = getTicks();
        if (now - last >= FRAME_TICKS)
        {
            last = now;
            gfxClear((Color){0, 128, 128});
            wmUpdate();
            wmDraw();
            taskbarDraw();
            drawMouse();
            gfxFlush();
        }
        __asm__ volatile("hlt");
    }
}