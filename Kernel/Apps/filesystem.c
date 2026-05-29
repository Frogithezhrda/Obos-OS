#include "filesystem.h"
#include "../Apps/textEditor.h"

static Label fileLabels[MAX_FILES];
Icon fileIcons[MAX_FILES];
static FileEntry fileEntries[MAX_FILES];
int fileCount = 0;
Icon fileIconFM;
Icon folderIconFM;
Button deleteIconFM;
static int popupVisible = 0;
static int popupIsDir   = 0;
static char popupBuffer[64];
static TextBox popupTextBox;
static Label popupTitleLabel;
int clickedFileIndex = -1;
static App* openEditorApp = NULL;

static void popupOnEnter(TextBox* tb)
{
    if (tb->label.text[0] != '\0')
    {
        if (popupIsDir)
        {
            createDir(tb->label.text);
        }
        else
        {
            createFile(tb->label.text, File);
        }
    }

    popupVisible = 0;

    openFileManager();
    setFocusedTextBox(0);
    redrawMouse();
}
static void deletePopupOnEnter(TextBox* tb)
{
    if (tb->label.text[0] != '\0')
    {
        deleteFile(tb->label.text);
    }

    popupVisible = 0;
    openFileManager();
    setFocusedTextBox(0);
    redrawMouse();
}

void showDeletePopup()
{
    popupVisible = 1;
    popupBuffer[0] = '\0';

    Window popup = createWindow(250, 200, 300, 120, (Color){210, 230, 150}, VISIBLE);
    Window popupBorder = createWindow(248, 198, 304, 124, DARK_GREY, VISIBLE);
    drawWindow(&popupBorder);
    drawWindow(&popup);

    Window titleWin = createWindow(250, 200, 300, 20, LIGHT_GREY, VISIBLE);
    popupTitleLabel = createLabel(&titleWin, "Delete File/Directory", 1, BLACK);
    drawLabel(&popupTitleLabel);

    Window nameLabelWin = createWindow(258, 230, 50, 16, (Color){210, 230, 150}, VISIBLE);
    Label nameLabel = createLabel(&nameLabelWin, "Name:", 1, BLACK);
    drawLabel(&nameLabel);

    Window tbWin = createWindow(320, 228, 210, 18, WHITE, VISIBLE);
    Label tbLabel = createLabel(&tbWin, popupBuffer, 1, BLACK);
    popupTextBox = createTextBox(&tbLabel, 63);
    popupTextBox.onEnter = deletePopupOnEnter;
    drawTextBox(&popupTextBox);
    setFocusedTextBox(&popupTextBox);

    Window hintWin = createWindow(258, 258, 280, 16, (Color){210, 230, 150}, VISIBLE);
    Label hintLabel = createLabel(&hintWin, "Press Enter to confirm", 1, BLACK);
    drawLabel(&hintLabel);

    redrawMouse();
}

void showCreatePopup(int isDir)
{
    popupIsDir = isDir;
    popupVisible = 1;
    popupBuffer[0] = '\0';

    Window popup = createWindow(250, 200, 300, 120, (Color){210, 230, 150}, VISIBLE);
    Window popupBorder = createWindow(248, 198, 304, 124, DARK_GREY, VISIBLE);
    drawWindow(&popupBorder);
    drawWindow(&popup);

    Window titleWin = createWindow(250, 200, 300, 20, LIGHT_GREY, VISIBLE);
    popupTitleLabel = createLabel(&titleWin, isDir ? "New Folder" : "New File", 1, BLACK);
    drawLabel(&popupTitleLabel);

    Window nameLabelWin = createWindow(258, 230, 50, 16, (Color){210, 230, 150}, VISIBLE);
    Label nameLabel = createLabel(&nameLabelWin, "Name:", 1, BLACK);
    drawLabel(&nameLabel);

    Window tbWin  = createWindow(310, 228, 220, 18, WHITE, VISIBLE);
    Label tbLabel = createLabel(&tbWin, popupBuffer, 1, BLACK);
    popupTextBox = createTextBox(&tbLabel, 63);
    popupTextBox.onEnter = popupOnEnter;
    drawTextBox(&popupTextBox);
    setFocusedTextBox(&popupTextBox);

    //hint
    Window hintWin = createWindow(258, 258, 280, 16, (Color){210, 230, 150}, VISIBLE);
    Label hintLabel = createLabel(&hintWin, "Press Enter to confirm", 1, (Color){80, 80, 80});
    drawLabel(&hintLabel);

    redrawMouse();
}

static void deleteGUI()
{
    showDeletePopup();
}

static void createDirGUI()
{
    showCreatePopup(1);
}

static void createFileGUI()
{
    showCreatePopup(0);
}

void openFileEditor()
{
    if(clickedFileIndex == -1) return;
    eraseMouse();
    App editorApp;
    editorApp.mainWin = createWindow(400, 150, 600, 400, (Color){255, 255, 240}, VISIBLE);
    editorApp.border = (Window){398, 148, 604, 404, DARK_GREY, VISIBLE};
    char buffer[128] = "Editor - ";

    strcat(buffer, fileLabels[clickedFileIndex].text);
    editorApp.bar = appBar(400, 150, 600, buffer);
    drawApp(&editorApp);
    editorOpen(&editorApp, fileLabels[clickedFileIndex].text);
    redrawMouse();

}

static void changeDir()
{
    if(clickedFileIndex == -1) return;
    cd(fileLabels[clickedFileIndex].text);
    clickedFileIndex = -1;
    openFileManager();
}

void drawCreationIcon(App* app)
{
    //100, 100, 600, 400
    openEditorApp = app;
    Window fileWin = createWindow(app->mainWin.x + 560, app->mainWin.y + 20, 40, 40, (Color){239, 255, 172}, VISIBLE);
    fileIconFM = createIcon(&fileWin, file16, createFileGUI);
    drawIcon(&fileIconFM, 2);
    Window folderWin = createWindow(app->mainWin.x + 530, app->mainWin.y + 20, 40, 40, (Color){239, 255, 172}, VISIBLE);
    folderIconFM = createIcon(&folderWin, folder16, createDirGUI);
    drawIcon(&folderIconFM, 2);
    Window deleteWin = createWindow(app->mainWin.x + 500, app->mainWin.y + 20, 20, 20, (Color){239, 255, 172}, VISIBLE);
    Label deleteLabel = createLabel(&deleteWin, "X", 3, RED);
    deleteIconFM = createButton(&deleteLabel, deleteGUI);
    drawButton(&deleteIconFM);
}

void drawFsFiles()
{
    fileCount = getDirectoryEntries(fileEntries, MAX_FILES);

    for (int i = 0; i < fileCount; i++)
    {
        int xDir = i / 6;
        int yDir = i % 6;

        int baseX = openEditorApp->mainWin.x + xDir * 180;
        int baseY = openEditorApp->mainWin.y + 40 + yDir * 60;
        
        unsigned char (*iconData)[16] = (fileEntries[i].type == Directory) ? folder16 : file16;

        Window iconWin = createWindow(baseX, baseY, 24, 24, (Color){239, 255, 188}, VISIBLE);

        fileIcons[i] = createIcon(&iconWin, iconData, (fileEntries[i].type == Directory) ? changeDir : openFileEditor);
        drawIcon(&fileIcons[i], 2);

        Window textWin = createWindow(baseX + 40, baseY + 12, 120, 16, (Color){239, 255, 188}, VISIBLE);

        fileLabels[i] = createLabel(&textWin, fileEntries[i].name, 1, BLACK);
        drawLabel(&fileLabels[i]);
    }
}