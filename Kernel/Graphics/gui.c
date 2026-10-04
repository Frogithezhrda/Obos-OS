#include "gui.h"
#include "../Desktop/desktop.h"
#include "../Desktop/wm.h"
#include "../Drivers/keyboardDriver.h"

unsigned int isGUIInitialized = 0;

void openGUI()
{
    isGUIInitialized = 1;
    gfxInit();
    gfxSetYield(keyboardPump);
    unsigned int last = getTicks();
    while (1)
    {
        int key = keyboardPoll();
        while (key != 0)
        {
            wmKey(key);
            key = keyboardPoll();
        }
        unsigned int now = getTicks();
        if (now - last >= FRAME_TICKS)
        {
            last = now;
            gfxClear((Color){0, 128, 128});
            wmUpdate();
            desktopUpdate();
            desktopDraw();
            wmDraw();
            taskbarDraw();
            drawMouse();
            gfxFlush();
        }
        __asm__ volatile("hlt");
    }
}