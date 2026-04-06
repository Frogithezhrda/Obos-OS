#include "paint.h"

static Window canvas;
static int isPainting = 0;

void openPaint()
{
    App editorApp;
    editorApp.mainWin = createWindow(100, 50, 600, 400, (Color){255, 255, 240}, VISIBLE);
    editorApp.border = (Window){98, 48, 604, 404, DARK_GREY, VISIBLE};

    editorApp.bar = appBar(100, 50, 600, "Paint");
    drawApp(&editorApp);

    canvas = createWindow(110, 80, 580, 350, WHITE, VISIBLE);
    drawWindow(&canvas);
}

void handlePaint(int mouseX, int mouseY)
{
    // check if inside canvas
    if (mouseX >= canvas.x && mouseX < canvas.x + canvas.width &&
        mouseY >= canvas.y && mouseY < canvas.y + canvas.height)
    {
        Pixel p = {mouseX, mouseY, BLACK};
        drawPixel(p);
    }
}
