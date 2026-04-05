#include "terminal.h"

static Terminal term;

unsigned int x;
unsigned int y;
unsigned int width;
unsigned int height;

#define PADDING      8
#define LINE_HEIGHT  20
#define MAX_LINES    20

static void terminalRedrawOutput()
{
    // clear output area
    Window outputArea = {x, y + 25, width, height - 50, BLACK, VISIBLE};
    drawWindow(&outputArea);

    // redraw all lines
    for (unsigned int i = 0; i < term.lineCount; i++)
    {
        term.outputLines[i].window.x = x + PADDING;
        term.outputLines[i].window.y       = y + 25 + i * LINE_HEIGHT;
        term.outputLines[i].window.width   = width - PADDING * 2;
        term.outputLines[i].window.height  = LINE_HEIGHT;
        term.outputLines[i].window.bgColor = BLACK;
        term.outputLines[i].window.isVisible = VISIBLE;
        term.outputLines[i].text           = term.lineBuffers[i];
        term.outputLines[i].textColor      = WHITE;
        term.outputLines[i].size           = 1;
        drawLabel(&term.outputLines[i]);
    }
}

static void terminalRedrawInput()
{
    // clear input bar
    Window inputArea = {x, y + 24, width, 24, (Color){30, 30, 30}, VISIBLE};
    drawWindow(&inputArea);

    char display[258];
    display[0] = '>';
    display[1] = ' ';
    unsigned int i;
    for (i = 0; i < term.inputLen; i++)
        display[i + 2] = term.inputBuffer[i];
    display[i + 2] = '_';
    display[i + 3] = '\0';

    Label inputLabel = {
        {x + PADDING, y + 22, width - PADDING * 2, 20, (Color){30, 30, 30}, VISIBLE},
        display, 1, GREEN
    };
    drawLabel(&inputLabel);
}

static void terminalPushLine(const char* text, Color color)
{
    if (term.lineCount >= MAX_LINES)
    {
        for (unsigned int i = 0; i < MAX_LINES - 1; i++)
        {
            unsigned int j = 0;
            while (term.lineBuffers[i + 1][j])
            {
                term.lineBuffers[i][j] = term.lineBuffers[i + 1][j];
                j++;
            }
            term.lineBuffers[i][j] = '\0';
            term.outputLines[i].textColor = term.outputLines[i + 1].textColor;
        }
        term.lineCount = MAX_LINES - 1;
    }

    unsigned int i = 0;
    while (text[i] && i < 63)
    {
        term.lineBuffers[term.lineCount][i] = text[i];
        i++;
    }
    term.lineBuffers[term.lineCount][i] = '\0';
    term.outputLines[term.lineCount].textColor = color;
    term.lineCount++;
    terminalRedrawOutput();
}


void terminalOpen(App* app)
{
    term.app = app;
    term.state = TerminalRunning;
    x = term.app->mainWin.x;
    y = term.app->mainWin.y;
    width = term.app->mainWin.width;
    height = term.app->mainWin.height;

    Window inputBar = createWindow(x, y + 24, width, 24, BLACK, VISIBLE);
    
    drawWindow(&inputBar);
    terminalPushLine("OBOS Terminal v1.0", CYAN);
    terminalPushLine("Type 'help' for commands.", LIGHT_GREY);

    terminalRedrawInput();
}

void terminalHandleKey(char c)
{

}