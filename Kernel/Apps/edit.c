#include "edit.h"

void editFile(const char* filename)
{
    char* buffer = (char*)kmalloc(200);
    int code = readFile(filename, buffer, 200);

    if(code == SUCCESS)
    {
        printLineW(buffer);
    }
    else
    {
        createFile(filename, File);
    }
    keybosIndex(buffer, 200, strlen(buffer));
    writeFile(filename, buffer, strlen(buffer));
    kfree(buffer);
}
void saveFile(const char* filename) 
{
    // Implementation for saving a file
}

void closeFile() 
{
    // Implementation for closing a file
}
