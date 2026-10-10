#ifndef MAIL_H
#define MAIL_H

#include "../SystemLib/util.h"
#include "../Drivers/keys.h"

void mailDraw(int x, int y, int w, int h);
void mailClick(int mouseX, int mouseY);
void mailKey(int key);
void mailPoll(unsigned int ticks);

#endif