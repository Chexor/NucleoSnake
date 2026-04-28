#include "snake_engine.h"

static uint16_t NextRandom(uint16_t *state) {
    uint16_t bit = (( (*state >> 0) ^ (*state >> 2) ^ (*state >> 3) ^ (*state >> 5) ) & 1U);
    *state = (uint16_t)((*state >> 1) | (bit << 15));
    return *state;
}

static bool SnakeContains(const Snake *snake, uint8_t x, uint8_t y) {
    for (uint8_t i = 0; i < snake->length; i++) {
        if (snake->x[i] == x && snake->y[i] == y) return true;
    }
    return false;
}

static void PlaceFood(GameState *state) {
    for (uint16_t attempt = 0; attempt < 512U; attempt++) {
        state->foodX = (uint8_t)(NextRandom(&state->randomState) % FIELD_W);
        state->foodY = (uint8_t)(NextRandom(&state->randomState) % FIELD_H);
        if (!SnakeContains(&state->snake, state->foodX, state->foodY)) return;
    }
    state->foodX = 0;
    state->foodY = 0;
}

void Snake_Reset(GameState *state, uint32_t seed) {
    state->snake.length = 5;
    state->snake.x[0] = 16; state->snake.y[0] = 8;
    state->snake.x[1] = 15; state->snake.y[1] = 8;
    state->snake.x[2] = 14; state->snake.y[2] = 8;
    state->snake.x[3] = 13; state->snake.y[3] = 8;
    state->snake.x[4] = 12; state->snake.y[4] = 8;
    
    state->snake.direction = DIR_RIGHT;
    state->nextDirection = DIR_RIGHT;
    state->turnQueued = false;
    state->score = 0;
    state->gameOver = false;
    state->randomState = (uint16_t)(seed ^ 0xACE1U);
    PlaceFood(state);
}

void Snake_TurnLeft(GameState *state) {
    switch (state->nextDirection) {
        case DIR_UP:    state->nextDirection = DIR_LEFT;  break;
        case DIR_LEFT:  state->nextDirection = DIR_DOWN;  break;
        case DIR_DOWN:  state->nextDirection = DIR_RIGHT; break;
        case DIR_RIGHT: state->nextDirection = DIR_UP;    break;
    }
}

void Snake_TurnRight(GameState *state) {
    switch (state->nextDirection) {
        case DIR_UP:    state->nextDirection = DIR_RIGHT; break;
        case DIR_RIGHT: state->nextDirection = DIR_DOWN;  break;
        case DIR_DOWN:  state->nextDirection = DIR_LEFT;  break;
        case DIR_LEFT:  state->nextDirection = DIR_UP;    break;
    }
}

void Snake_Move(GameState *state) {
    int8_t newX = (int8_t)state->snake.x[0];
    int8_t newY = (int8_t)state->snake.y[0];
    
    state->snake.direction = state->nextDirection;

    switch (state->snake.direction) {
        case DIR_UP:    newY--; break;
        case DIR_DOWN:  newY++; break;
        case DIR_LEFT:  newX--; break;
        case DIR_RIGHT: newX++; break;
    }

    if (newX < 0 || newX >= FIELD_W || newY < 0 || newY >= FIELD_H) {
        state->gameOver = true;
        return;
    }

    bool grow = ((uint8_t)newX == state->foodX && (uint8_t)newY == state->foodY);

    for (uint8_t i = 0; i < state->snake.length; i++) {
        if (state->snake.x[i] == (uint8_t)newX && state->snake.y[i] == (uint8_t)newY) {
            if (grow || (i != (state->snake.length - 1))) {
                state->gameOver = true;
                return;
            }
        }
    }

    if (grow && state->snake.length < MAX_SNAKE_LENGTH) {
        state->snake.length++;
        state->score++;
    }

    for (int16_t i = (int16_t)state->snake.length - 1; i > 0; i--) {
        state->snake.x[i] = state->snake.x[i - 1];
        state->snake.y[i] = state->snake.y[i - 1];
    }

    state->snake.x[0] = (uint8_t)newX;
    state->snake.y[0] = (uint8_t)newY;

    if (grow) PlaceFood(state);
    state->turnQueued = false;
}
