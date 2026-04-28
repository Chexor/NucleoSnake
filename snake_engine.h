#ifndef SNAKE_ENGINE_H
#define SNAKE_ENGINE_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_SNAKE_LENGTH    160U
#define FIELD_W             31U  // Derived from (128 - 2*2) / 4
#define FIELD_H             15U  // Derived from (64 - 2*2) / 4

typedef enum {
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef struct {
    uint8_t x[MAX_SNAKE_LENGTH];
    uint8_t y[MAX_SNAKE_LENGTH];
    uint8_t length;
    Direction direction;
} Snake;

typedef struct {
    Snake snake;
    uint8_t foodX;
    uint8_t foodY;
    uint16_t score;
    Direction nextDirection;
    bool turnQueued;
    bool gameOver;
    uint16_t randomState;
} GameState;

void Snake_Reset(GameState *state, uint32_t seed);
void Snake_Move(GameState *state);
void Snake_TurnLeft(GameState *state);
void Snake_TurnRight(GameState *state);

#endif // SNAKE_ENGINE_H
