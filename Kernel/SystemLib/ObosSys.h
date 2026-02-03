#ifndef OBOSSYS_H
#define OBOSSYS_H


void printo(const char* string, unsigned int color);
//TODO
// unsigned int getTicks();
// void sleep(const unsigned int ms);



#define printoW(string) printo(string, 0)
#endif