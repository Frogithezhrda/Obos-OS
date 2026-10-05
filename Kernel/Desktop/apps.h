#ifndef APPS_H
#define APPS_H
#include "../SystemLib/util.h"

int appCount(void);
const char* appName(int i);
unsigned char (*appIcon(int i))[16];
void appLaunch(int i);
void appLaunchByName(const char* name);

#endif