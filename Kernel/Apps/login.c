#include "login.h"

User users[2];

void loadUsers()
{
    char* buffer = (char*)kmalloc(256);
    if (readFile("users.dat", buffer, 256) == ERROR)
    {
        printLineW("Failed to load users!");
        kfree(buffer);
        return;
    }
    strcpy(users[0].username, strtok(buffer, "&"));
    users[0].salt = atoi(strtok(NULL, "&"));
    users[0].hash = atoi(strtok(NULL, "\n"));
    strcpy(users[1].username, strtok(NULL, "&"));
    users[1].salt = atoi(strtok(NULL, "&"));
    users[1].hash = atoi(strtok(NULL, "\n"));

    kfree(buffer);
}

unsigned int generateSalt()
{
    srand(getTicks());
    unsigned int salt = 0;
    //random bullshit, not really random but good enough for our purposes
    salt = rand();
    salt = (salt << 16) ^ rand();
    return salt;
}

unsigned int hashPassword(const char* password, unsigned int salt)
{
    unsigned int hash = salt;

    for (int round = 0; round < ROUND_COUNT; round++) 
    {
        const char* p = password;

        while (*p) 
        {
            hash = hash * 17 + *p;
            p++;
        }

        hash ^= salt;
    }

    return hash;
}

int loginMenu()
{
    loadUsers();
    char username[USER_LENGTH];
    char password[PASS_LENGTH];
    for(int attempt = 0; attempt < 3; attempt++)
    {
        printW("Username: ");
        keybos(username, USER_LENGTH);

        printW("Password: ");
        keybos(password, PASS_LENGTH);

        int found = 0;

        for(unsigned int j = 0; j < 2; j++)
        {
            User* user = &users[j];

            if (strcmp(username, user->username) == 0) 
            {
                found = 1;

                unsigned int hashedInput = hashPassword(password, user->salt);

                if (hashedInput == user->hash) 
                {
                    printLineW("Login success!");
                    printW("Welcome, ");
                    printLineW(user->username);
                    return SUCCESS;
                }
                else 
                {
                    printLineW("Wrong password!");
                    break;
                }
            }
        }

        if (!found)
        {
            printLineW("Wrong username!");
        }
    }
    printLineW("User not found");
    return ERROR;
}