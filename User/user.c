#include "ObosSys.h"

void obm()
{
    char buffer[64];
    char cbuf[2];
    int i = 0;
    printo("type: ", 0);
    while (1)
    {
        if (inputo(cbuf, 1))
        {
            char c = cbuf[0];

            if (c == '\n')
                break;
            
            buffer[i++] = c;
        }
    }

    buffer[i] = '\0';

    printoW(buffer);

    while(1);
}
