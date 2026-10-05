#ifndef UTIL_H
#define UTIL_H
 
int strLen(const char* s);
void strAppend(char* dst, const char* src);
void utoa10(unsigned int v, char* out);
int startsWith(const char* s, const char* prefix);
int clampInt(int v, int lo, int hi);
 
#endif
