#ifndef BROWSER_H
#define BROWSER_H

#include "../Network/tcp.h"

void browserInit();
void browserOpen(const char* url);
void browserPoll(unsigned int ticks);
void browserDraw(int x, int y, int w, int h);
void browserClick(int x, int y);
void browserKey(int key);
void browserScroll(int lines);
const char* browserTitle();

#endif