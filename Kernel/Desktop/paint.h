#ifndef PAINT_H
#define PAINT_H

#define PAINT_MIN_W 400
#define PAINT_MIN_H 282

void paintDraw(int x, int y, int w, int h);
void paintClick(int lx, int ly);
void paintDrag(int lx, int ly);
void paintRelease(void);
void paintKey(int key);

#endif