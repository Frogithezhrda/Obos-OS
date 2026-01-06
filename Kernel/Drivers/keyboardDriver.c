#include "keyboardDriver.h"

//keys table
static const char scancodeToASCII[LAST_SCAN_CODE] = 
{
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'
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

static char keyboardBuffer[KEYBOARD_BUFFER_SIZE];
static unsigned int bufferIndex = 0;
static unsigned char shiftPressed = 0; //flag for shift pressed

void keyboardISR(void)
{
    unsigned char asciiChar;
    unsigned char scanCode = inb(KEYBOARD_SCAN_CODE_PORT); // reading from keyboard data port
    if (!(scanCode & KEYPRESS_MASK)) //key press event
    {
        if(scanCode == SHIFT_LEFT_PRESS || scanCode == SHIFT_RIGHT_PRESS) 
        {
            shiftPressed = 1;
        }
        else if (scanCode < LAST_SCAN_CODE) //valid scan code
        {
            if (shiftPressed)
            {
                asciiChar = scancodeToASCIIShift[scanCode];
            }
            else
            {
                asciiChar = scancodeToASCII[scanCode];
            }
            if(asciiChar == '\b') //backspace handling
            {
                deleteChar();
            }
            else if (asciiChar != 0 && bufferIndex < KEYBOARD_BUFFER_SIZE - 1)
            {
                keyboardBuffer[bufferIndex++] = asciiChar;
                keyboardBuffer[bufferIndex] = '\0'; //null-terminate the string
                //printing the char
            }
            clearScreen();
            print(keyboardBuffer, WHITE);
        }
    }
    else 
    {
        if (scanCode == SHIFT_LEFT_RELEASE || scanCode == SHIFT_RIGHT_RELEASE)
        {
            shiftPressed = 0;
        }
    }
    endOfInterrupt(1); //IRQ1 is keyboard
}

//half works needs the console driver to support backspace properly
void deleteChar(void)
{
    if (bufferIndex > 0)
    {
        //works as half baked solution
        bufferIndex--;
        keyboardBuffer[bufferIndex] = ' ';
        keyboardBuffer[bufferIndex + 1] = '\0';
        //printing the updated buffer
        clearScreen();
        print(keyboardBuffer, WHITE);
    }
}