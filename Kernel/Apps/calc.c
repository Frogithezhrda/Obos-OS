#include "calc.h"

void menu()
{
    char option = NULL;
    printW("Calculator Menu:\n");
    printW("1. Add\n");
    printW("2. Subtract\n");
    printW("3. Multiply\n");
    printW("4. Divide\n");
    printW("Select an option: ");
    while(option != 0) option = keybosChar();
    switch(option)
    {
        case '1':

            break;
        case '2':
            // Handle subtraction
            break;
        case '3':
            // Handle multiplication
            break;
        case '4':
            // Handle division
            break;
        default:
            printW("Invalid option. Please try again.\n");
            menu();
            break;
    }
}

int add(int a, int b)
{
    return a + b;
}

int sub(int a, int b)
{
    return a - b;
}

int mul(int a, int b)
{
    return a * b;
}

int div(int a, int b)
{
    if(b != 0)
    {
        return a / b;
    }
    return 0; // Handle division by zero
}
