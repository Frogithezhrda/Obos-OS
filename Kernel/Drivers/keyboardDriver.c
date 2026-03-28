#include "keyboardDriver.h"

// Keys table
static const char scancodeToASCII[LAST_SCAN_CODE] = 
{
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '7', '8', 'v', '-', '4', '>', '6', '<', '1', '2', 'V', '0', '.'
};

static const char scancodeToASCIIShift[LAST_SCAN_CODE] = 
{
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'
};

static volatile unsigned char shiftPressed = 0;
static volatile unsigned char lastScanCode = 0;

void keyboardISR(void)
{
    lastScanCode = inb(KEYBOARD_SCAN_CODE_PORT);
    endOfInterrupt(1);
}

void deleteChar(void)
{
    printW("\b \b");
}

void keybosIndex(char* string, const int maxLength, int index)
{
    if (index < 0 || index >= maxLength) return;
    unsigned char scanCode;
    unsigned char asciiChar;
    //clearing the string
    // string[0] = '\0';
    
    while (1)
    {
        while (lastScanCode == 0);
        
        scanCode = lastScanCode;
        lastScanCode = 0;
        //72 75 77 80
        if (scanCode & KEYPRESS_MASK)
        {
            if (scanCode == SHIFT_LEFT_RELEASE || scanCode == SHIFT_RIGHT_RELEASE)
            {
                shiftPressed = 0;
            }
            continue;
        }
        
        if (scanCode == SHIFT_LEFT_PRESS || scanCode == SHIFT_RIGHT_PRESS)
        {
            shiftPressed = 1;
            continue;
        }
        
        if (scanCode >= LAST_SCAN_CODE) 
            continue;
        
        if (shiftPressed)
        {
            asciiChar = scancodeToASCIIShift[scanCode];
        }
        else
        {
            asciiChar = scancodeToASCII[scanCode];
        }
        
        if (asciiChar == 0) 
            continue;
        
        if (asciiChar == '\n')
        {
            string[index] = '\0';
            printLineW("");
            break;
        }
        else if (asciiChar == '\b')
        {
            if (index > 0)
            {
                index--;
                string[index] = '\0';
                deleteChar();
            }
        }
        else if (index < maxLength - 1)
        {
            string[index] = asciiChar;
            index++;
            string[index] = '\0';
            printChar(asciiChar, LIGHT_CYAN);
        }
        
    }

}

char keybosChar()
{
    unsigned char scanCode;
    unsigned char asciiChar;

    while (1)
    {
        while (lastScanCode == 0) return 0;

        scanCode = lastScanCode;
        lastScanCode = 0;

        if (scanCode >= LAST_SCAN_CODE)
        {
            return 0;
        }
        asciiChar = scancodeToASCII[scanCode];

        if (asciiChar == 0)
        {
            return 0;
        }
        return asciiChar;
    }
    return 0;
}

void keybos(char* string, const int maxLength)
{
    keybosIndex(string, maxLength, 0);
}