#ifndef LOGIN_H
#define LOGIN_H

#define ROUND_COUNT 1000
#define USER_LENGTH 10
#define PASS_LENGTH 12
#define SUCCESS 0
#define ERROR -1

#include "../Drivers/timerDriver.h"
#include "../SystemLib/obosMemory.h"

typedef struct User
{
    char username[USER_LENGTH];
    unsigned int hash;
    unsigned int salt;
} User;

void loadUsers();
unsigned int generateSalt();
unsigned int hashPassword(const char* password, unsigned int salt);
int loginMenu();

#endif