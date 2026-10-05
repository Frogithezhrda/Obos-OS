#ifndef EDITOR_H
#define EDITOR_H
 #include "../SystemLib/util.h"

const char* editorOpen(const char* name, unsigned int size);
void editorDraw(int x, int y, int w, int h);
void editorClick(int lx, int ly);
void editorKey(int key);
 void editorRenamed(const char* oldName, const char* newName);
void editorMoved(const char* name, unsigned int oldDir, unsigned int newDir);
#endif
