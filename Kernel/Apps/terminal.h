#ifndef TERMINAL_H
#define TERMINAL_H

#include "../Graphics/apps.h"

typedef enum 
{
    TerminalClosed  = 0,
    TerminalRunning = 1
} TerminalState;

typedef struct Terminal
{
    char inputBuffer[256];
    unsigned int inputLen;
    TerminalState state;
    Label outputLines[20];
    char lineBuffers[20][64];
    unsigned int lineCount;
    App* app;
} Terminal;



void terminalOpen(App* app);

#endif