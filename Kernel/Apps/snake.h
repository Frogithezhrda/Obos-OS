#ifndef SNAKE_H
#define SNAKE_H

#include "../Fs/superblock.h"
#include "../Memory/heap.h"
#include "../Drivers/keyboardDriver.h"
#include "../Drivers/timerDriver.h"
#define NUM_TILES (16 * 16)
#define MAX_SNAKE_LENGTH 32

typedef enum Direction {UP, DOWN, LEFT, RIGHT};
typedef enum TileType {EMPTY, SNAKE_BODY, SNAKE_HEAD, FOOD, WALL};
typedef enum Difficulty {EASY = 30, MEDIUM = 20, HARD = 10};
typedef struct {
    int x;
    int y;
} SnakeSegment;

typedef struct Snake {
    int x;
    int y;
    int length;
    char head;
    int isDead;
    enum Direction direction;
    SnakeSegment body[MAX_SNAKE_LENGTH];
} Snake;

typedef struct Food {
    int x;
    int y;
} Food;

typedef struct Tile {
    int x;
    int y;
    enum TileType type;
} Tile;
void runSnakeGame();
void snakeInit();
void snakeUpdate(const enum Direction dir);
void snakeRender();
void placeFood();
void setDiff(enum Difficulty diff);

Tile tiles[NUM_TILES];

#endif // SNAKE_H