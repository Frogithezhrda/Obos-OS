#include "calc.h"

void menu()
{
    char option = NULL;
    while(1)
    {
        char buffA[12];
        char buffB[12];
        char result[1];
        printW("Calculator Menu:\n");
        printW("1. Add\n");
        printW("2. Subtract\n");
        printW("3. Multiply\n");
        printW("4. Divide\n");
        printW("5. Bye\n");
        printW("Select an option: ");
        keybos(result, 2);
        option = result[0];
        if(option == '5')
        {
            printW("Exiting Calculator App...\n");
            return;
        }
        printW("Enter first number: ");
        keybos(buffA, 12);
        printW("Enter second number: ");
        keybos(buffB, 12);
        int a = atoi(buffA);
        int b = atoi(buffB);
        printW("Result:");
        switch(option)
        {
            case '1':
                printNumberW(add(a, b));
                break;
            case '2':
                printNumberW(sub(a, b));
                break;
            case '3':
                printNumberW(mul(a, b));
                break;
            case '4':
                printNumberW(div(a, b));
                break;
            default:
                printW("Invalid option. Please try again.\n");
                menu();
                break;
        }
        printW("\n");
        printW("\n");
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
    return 0;
}
