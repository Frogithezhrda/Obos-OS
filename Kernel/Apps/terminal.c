#include "terminal.h"

static Terminal term;
static Label labelStart;
unsigned int x;
unsigned int y;
unsigned int width;
unsigned int height;

#define PADDING      8
#define LINE_HEIGHT  20
#define MAX_LINES    18

//terminal works now we just need to build on it

static TextBox inputTextBox;

static void terminalRedrawOutput()
{
    // clear output area
    Window outputArea = {x, y + 25, width, height - 50, BLACK, VISIBLE};
    drawWindow(&outputArea);

    // redraw all lines
    for (unsigned int i = 0; i < term.lineCount; i++)
    {
        term.outputLines[i].window.x = x + PADDING;
        term.outputLines[i].window.y = y + 25 + i * LINE_HEIGHT;
        term.outputLines[i].window.width = width - PADDING * 2;
        term.outputLines[i].window.height = LINE_HEIGHT;
        term.outputLines[i].window.bgColor = BLACK;
        term.outputLines[i].window.isVisible = VISIBLE;
        term.outputLines[i].text = term.lineBuffers[i];
        // term.outputLines[i].textColor = WHITE;
        term.outputLines[i].size = 1;
        drawLabel(&term.outputLines[i]);
    }

    Window inputBar = createWindow(x, y + 24, width, 24, BLACK, VISIBLE);
    drawWindow(&inputBar);
    drawLabel(&labelStart);
    drawTextBox(&inputTextBox);
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

static void terminalShell(char* str)
{
    if(!strcmp("help", str))
    {
        terminalPushLine("help - help command", LIGHT_CYAN);
        terminalPushLine("clear - clears the screen", LIGHT_CYAN);
    }     
    else if (!strcmp("clear", str))
    {
        term.lineCount = 1;
        terminalRedrawOutput();
    }
    else
    {
        terminalPushLine("Unknown Command!", RED);
    }
}

static void terminalOnEnter(TextBox* tb)
{
    // terminalPushLine(tb->label.text, WHITE);
    terminalShell(tb->label.text);

    tb->cursorPos = 0;
    tb->label.text[0] = '\0';
    drawTextBox(tb);
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
    term.lineCount = 1;
    terminalRedrawOutput();
    terminalPushLine("", CYAN);
    terminalPushLine("OBOS Terminal v1.0", CYAN);
    terminalPushLine("Type 'help' for commands.", LIGHT_GREY);
    term.inputBuffer[0] = '\0';
    Window textWin = createWindow(x + 24, y + 24, width - 24, 24, BLACK, VISIBLE);
    Label labelTxt = createLabel(&textWin, term.inputBuffer, 1, WHITE);
    Window startWin = createWindow(x, y + 24, width, 24, BLACK, VISIBLE);
    labelStart = createLabel(&startWin, ">>", 1, LIGHT_GREEN);
    inputTextBox = createTextBox(&labelTxt, 40);
    inputTextBox.onEnter = terminalOnEnter;
    drawTextBox(&inputTextBox);
    drawLabel(&labelStart);
    setFocusedTextBox(&inputTextBox);
    // terminalRedrawInput();
}
