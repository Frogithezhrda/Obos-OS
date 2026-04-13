#include "textEditor.h"
#include "filesystem.h"

#define EDITOR_MAX_LINES  20
#define EDITOR_LINE_LEN   64
#define EDITOR_LINE_H     16
#define EDITOR_PADDING    8

static unsigned int ex, ey, ew, eh;
static char editorLines[EDITOR_MAX_LINES][EDITOR_LINE_LEN];
static Label editorLabels[EDITOR_MAX_LINES];
static unsigned int editorLineCount = 1;
static unsigned int editorCurrentLine = 0;
static char editorFilename[64];
static TextBox editorInput;
static char editorInputBuf[EDITOR_LINE_LEN];

static void editorRedraw()
{
    //clearing content area
    Window content = createWindow(ex, ey, ew, eh - 28, (Color){255, 255, 240}, VISIBLE);
    drawWindow(&content);
    for (unsigned int i = 0; i < editorLineCount; i++)
    {
        Window lw = createWindow(ex + EDITOR_PADDING, ey + EDITOR_PADDING + i * EDITOR_LINE_H, ew - EDITOR_PADDING * 2, EDITOR_LINE_H, (Color){255, 255, 240}, VISIBLE );
        editorLabels[i] = createLabel(&lw, editorLines[i], 1, BLACK);
        drawLabel(&editorLabels[i]);
    }

    //redrawing input bar
    drawTextBox(&editorInput);
}

static void editorOnEnter(TextBox* tb)
{

    //saving current lines
    unsigned int i = 0;
    while (tb->label.text[i] && i < EDITOR_LINE_LEN - 1)
    {
        editorLines[editorCurrentLine][i] = tb->label.text[i];
        i++;
    }
    editorLines[editorCurrentLine][i] = '\0';

    //write all lines to file
    char fileBuffer[EDITOR_MAX_LINES * EDITOR_LINE_LEN];
    unsigned int pos = 0;
    for (unsigned int l = 0; l <= editorCurrentLine; l++)
    {
        unsigned int j = 0;
        while (editorLines[l][j] && pos < sizeof(fileBuffer) - 1)
        {
            fileBuffer[pos] = editorLines[l][j++];
            pos++;
        }
        fileBuffer[pos] = '\n';
        pos++;
    }
    fileBuffer[pos] = '\0';
    writeFile(editorFilename, fileBuffer, pos);

    // move to next line
    if (editorCurrentLine < EDITOR_MAX_LINES - 1)
    {
        editorCurrentLine++;
    }

    if (editorCurrentLine >= editorLineCount)
    {
        editorLineCount = editorCurrentLine + 1;
    }

    //clearing input fr next line
    tb->cursorPos = 0;
    tb->label.text[0] = '\0';

    editorRedraw();
}
void editorOpen(App* app, const char* filename)
{
    ex = app->mainWin.x;
    ey = app->mainWin.y + 20;
    ew = app->mainWin.width;
    eh = app->mainWin.height - 20; 

    strcpy(editorFilename, filename);

    //clear lines
    editorLineCount = 1;
    editorCurrentLine = 0;
    for (unsigned int l = 0; l < EDITOR_MAX_LINES; l++)
    {
        editorLines[l][0] = '\0';
    }

    //read
    char fileBuffer[EDITOR_MAX_LINES * EDITOR_LINE_LEN];
    fileBuffer[0] = '\0';
    readFile(filename, fileBuffer, sizeof(fileBuffer) - 1);

    //putting to lines
    unsigned int lineIdx = 0;
    unsigned int colIdx = 0;
    for (unsigned int i = 0; fileBuffer[i] != '\0' && lineIdx < EDITOR_MAX_LINES; i++)
    {
        if (fileBuffer[i] == '\n')
        {
            editorLines[lineIdx][colIdx] = '\0';
            lineIdx++;
            colIdx = 0;
        }
        else if (colIdx < EDITOR_LINE_LEN - 1)
        {
            editorLines[lineIdx][colIdx] = fileBuffer[i];
            colIdx++;
        }
    }
    if (colIdx > 0)
    {
        editorLines[lineIdx][colIdx] = '\0';
        lineIdx++;
    }

    editorLineCount = lineIdx > 0 ? lineIdx : 1;
    editorCurrentLine = editorLineCount;

    //clear content area
    Window content = createWindow(ex, ey, ew, eh, (Color){255, 255, 240}, VISIBLE);
    drawWindow(&content);

    //separator above input
    Window sep = createWindow(ex, ey + eh - 28, ew, 2, DARK_GREY, VISIBLE);
    drawWindow(&sep);

    //input bar at bottom(to much works for above currently on bottom)
    editorInputBuf[0] = '\0';
    Window tbWin = createWindow(ex + 4, ey + eh - 26, ew - 8, 24, WHITE, VISIBLE);
    Label tbLabel = createLabel(&tbWin, editorInputBuf, 1, BLACK);
    editorInput = createTextBox(&tbLabel, EDITOR_LINE_LEN);
    editorInput.onEnter = editorOnEnter;
    drawTextBox(&editorInput);
    setFocusedTextBox(&editorInput);

    editorRedraw();
}