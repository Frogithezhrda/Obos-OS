#include "keyboardDriver.h"

// Keys table
static const char scancodeToASCII[LAST_SCAN_CODE] = 
{
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '7', '8', 'v', '-', '4', '<', '6', '>', '1', '2', 'V', '0', '.'
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
static int keyQueue[KEY_QUEUE_SIZE];
static int keyHead = 0;
static int keyTail = 0;
static int extended = 0;
static int ctrlPressed = 0;

static void pushKey(int key)
{
    int next = (keyHead + 1) % KEY_QUEUE_SIZE;
    if (next == keyTail) return;
    keyQueue[keyHead] = key;
    keyHead = next;
}

void keyboardPump(void)
{
    unsigned char sc = lastScanCode;
    if (sc == 0) return;
    lastScanCode = 0;
    if (sc == 0xE0)
    {
        extended = 1;
        return;
    }
    int isExtended = extended;
    extended = 0;
    int release = sc & KEYPRESS_MASK;
    unsigned char code = sc & 0x7F;
    if (isExtended && (code == 0x2A || code == 0x36)) return;
    if (code == 0x1D)
    {
        ctrlPressed = !release;
        return;
    }
    if (sc == SHIFT_LEFT_PRESS || sc == SHIFT_RIGHT_PRESS)
    {
        shiftPressed = 1;
        return;
    }
    if (sc == SHIFT_LEFT_RELEASE || sc == SHIFT_RIGHT_RELEASE)
    {
        shiftPressed = 0;
        return;
    }
    if (release) return;
    if (isExtended)
    {
        switch (code)
        {
            case 0x48: pushKey(KEY_UP); break;
            case 0x50: pushKey(KEY_DOWN); break;
            case 0x4B: pushKey(KEY_LEFT); break;
            case 0x4D: pushKey(KEY_RIGHT); break;
            case 0x47: pushKey(KEY_HOME); break;
            case 0x4F: pushKey(KEY_END); break;
            case 0x53: pushKey(KEY_DELETE); break;
            case 0x1C: pushKey(KEY_ENTER); break;
        }
        return;
    }
    if (ctrlPressed)
    {
        if (code == 0x1F) pushKey(KEY_SAVE);
        return;
    }
    if (code > 57) return;
    char ascii = shiftPressed ? scancodeToASCIIShift[code] : scancodeToASCII[code];
    if (ascii != 0) pushKey((unsigned char)ascii);
}

int keyboardPoll(void)
{
    keyboardPump();
    if (keyTail == keyHead) return 0;
    int key = keyQueue[keyTail];
    keyTail = (keyTail + 1) % KEY_QUEUE_SIZE;
    return key;
}

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
        if (scanCode & 0x80)
        {
            return 0;
        }
        if (scanCode >= LAST_SCAN_CODE)
        {
            return 0;
        }
        asciiChar = scancodeToASCII[scanCode];

        if (asciiChar == 0)
        {
            return 0;
        }
        printChar(asciiChar, WHITE);
        return asciiChar;
    }
    return 0;
}

void keybos(char* string, const int maxLength)
{
    keybosIndex(string, maxLength, 0);
}

// void keybosGUI(TextBox* tb)
// {
//     if (lastScanCode == 0) return;  // nothing to process

//     unsigned char scanCode = lastScanCode;
//     lastScanCode = 0;

//     if (scanCode & KEYPRESS_MASK)
//     {
//         if (scanCode == SHIFT_LEFT_RELEASE || scanCode == SHIFT_RIGHT_RELEASE)
//             shiftPressed = 0;
//         return;
//     }

//     if (scanCode == SHIFT_LEFT_PRESS || scanCode == SHIFT_RIGHT_PRESS)
//     {
//         shiftPressed = 1;
//         return;
//     }

//     if (scanCode >= LAST_SCAN_CODE) return;

//     unsigned char asciiChar = shiftPressed
//         ? scancodeToASCIIShift[scanCode]
//         : scancodeToASCII[scanCode];

//     if (asciiChar == 0)   return;
//     if (asciiChar == '\n')
//     {
//         if (tb->onEnter) tb->onEnter(tb);
//         return;
//     }

//     if (asciiChar == '\b')
//     {
//         if (tb->cursorPos > 0)
//         {
//             tb->cursorPos--;
//             tb->label.text[tb->cursorPos] = '\0';
//             drawTextBox(tb);
//         }
//     }
//     else if (tb->cursorPos < tb->maxSize - 1)
//     {
//         tb->label.text[tb->cursorPos++] = asciiChar;
//         tb->label.text[tb->cursorPos]   = '\0';
//         drawTextBox(tb);
//     }
// }