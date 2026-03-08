#include "snake.h"

Snake snake;
unsigned int speed = MEDIUM;

void setDiff(enum Difficulty diff)
{
    speed = diff;
}

void runSnakeGame()
{
    snakeInit();
    while(1)
    {
        unsigned int lastUpdate = getTicks();

        char input = keybosChar();

        if (input == 'w') snake.direction = UP;
        else if (input == 's') snake.direction = DOWN;
        else if (input == 'a') snake.direction = LEFT;
        else if (input == 'd') snake.direction = RIGHT;

        if (lastUpdate % speed == 0)
        {
            snakeUpdate(snake.direction);
            sleep(10);
            if (snake.x < 1 || snake.x >= 15 || snake.y < 1 || snake.y >= 15
                || snake.isDead)            
            {
            snakeRender();
            sleep(1000);
            clearScreen();
            if(snake.length == MAX_SNAKE_LENGTH)
            {
                print("Congratulations! You win!", LIGHT_GREEN);
                setDiff(MEDIUM);
                break;
            }
            printLine("You Lost. Game Over!", LIGHT_RED);
            setDiff(MEDIUM);
            break;
            }
            snakeRender();
        }
    }
}

void snakeInit()
{
    srand();
    for(unsigned int i = 0; i < NUM_TILES; i++) 
    {
        tiles[i].x = i % 16;
        tiles[i].y = i / 16;
        tiles[i].type = EMPTY;
        if(tiles[i].x == 0 || tiles[i].x == 15 || tiles[i].y == 0 || tiles[i].y == 15)
        {
            tiles[i].type = WALL;
        }
    }
    snake.x = 8;
    snake.y = 8;
    snake.body[0].x = snake.x;
    snake.body[0].y = snake.y;
    snake.length = 1;
    snake.direction = RIGHT;
    snake.head = '>';
    tiles[snake.y * 16 + snake.x].type = SNAKE_HEAD;
    placeFood();
    snakeRender();
}

void snakeUpdate(const enum Direction dir)
{
    snake.direction = dir;

    SnakeSegment oldTail = snake.body[snake.length - 1];

    for (int i = snake.length - 1; i > 0; i--)
    {
        snake.body[i] = snake.body[i - 1];
    }

    // 3. move head
    switch (dir)
    {
        case UP:    snake.head = '^'; snake.body[0].y--; break;
        case DOWN:  snake.head = 'v'; snake.body[0].y++; break;
        case LEFT:  snake.head = '<'; snake.body[0].x--; break;
        case RIGHT: snake.head = '>'; snake.body[0].x++; break;
    }

    snake.x = snake.body[0].x;
    snake.y = snake.body[0].y;

    int atFood = (tiles[snake.y * 16 + snake.x].type == FOOD);
    int atBody = (tiles[snake.y * 16 + snake.x].type == SNAKE_BODY);

    tiles[oldTail.y * 16 + oldTail.x].type = EMPTY;

    for (unsigned int i = 1; i < snake.length; i++)
    {
        tiles[snake.body[i].y * 16 + snake.body[i].x].type = SNAKE_BODY;
    }

    tiles[snake.y * 16 + snake.x].type = SNAKE_HEAD;

    snake.isDead = atBody;

    if (atFood)
    {
        if (snake.length < MAX_SNAKE_LENGTH)
        {
            snake.length++;
        }
        else
        {
            snake.isDead = 1;
        }
        placeFood();
    }

}

void snakeRender()
{
    clearScreen();
    for(unsigned int i = 0; i < NUM_TILES; i++) 
    {
        if(tiles[i].type == WALL)
        {
            print("#", YELLOW);
        }
        else if (tiles[i].type == SNAKE_BODY)
        {
            print("o", LIGHT_GREEN);
        }
        else if (tiles[i].type == SNAKE_HEAD) 
        {
            printChar(snake.head, GREEN);
        } 
        else if (tiles[i].type == FOOD) 
        {
            print("X", LIGHT_RED);
        }
        else 
        {
            print(".", LIGHT_GREY);
        }
        
        if (tiles[i].x == 15) printW("\n");
    }
    printW("Score: ");
    printNumberW(snake.length - 1);
}

void placeFood()
{
    unsigned int x, y;
    do 
    {
        x = randRange(1, 14);
        y = randRange(1, 14);
    } while (tiles[y * 16 + x].type != EMPTY);
    tiles[y * 16 + x].type = FOOD;
}