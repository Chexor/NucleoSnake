/**
 * @file snake_engine.c
 * @brief Implementation of Snake game mechanics and pseudo-randomness.
 */

#include "snake_engine.h"

/**
 * @brief Linear Feedback Shift Register (LFSR) for pseudo-random number generation.
 * 
 * CONCEPT: An LFSR is an efficient way to generate a sequence of bits that 
 * appears random. It uses a "tap" system (XORing specific bits) to produce 
 * the next bit in the sequence. This is ideal for microcontrollers as it 
 * avoids heavy mathematical libraries.
 * 
 * @param state Pointer to the 16-bit LFSR state.
 * @return Next pseudo-random value.
 */
static uint16_t NextRandom(uint16_t *state) {
    /* Taps at positions 0, 2, 3, and 5 for a 16-bit LFSR */
    uint16_t bit = (( (*state >> 0) ^ (*state >> 2) ^ (*state >> 3) ^ (*state >> 5) ) & 1U);
    *state = (uint16_t)((*state >> 1) | (bit << 15));
    return *state;
}

/**
 * @brief Checks if a coordinate is occupied by any part of the snake body.
 */
static bool SnakeContains(const Snake *snake, uint8_t x, uint8_t y) {
    for (uint8_t i = 0; i < snake->length; i++) {
        if (snake->x[i] == x && snake->y[i] == y) return true;
    }
    return false;
}

/**
 * @brief Places food at a random unoccupied position on the field.
 */
static void PlaceFood(GameState *state) {
    for (uint16_t attempt = 0; attempt < 512U; attempt++) {
        state->foodX = (uint8_t)(NextRandom(&state->randomState) % FIELD_W);
        state->foodY = (uint8_t)(NextRandom(&state->randomState) % FIELD_H);
        if (!SnakeContains(&state->snake, state->foodX, state->foodY)) return;
    }
    /* Fallback to (0,0) if field is extremely crowded */
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
    
    /* Seed the LFSR with system ticks for initial variety */
    state->randomState = (uint16_t)(seed ^ 0xACE1U);
    PlaceFood(state);
}

void Snake_TurnLeft(GameState *state) {
    /* Maps relative 'Left' to cardinal directions based on current heading */
    switch (state->nextDirection) {
        case DIR_UP:    state->nextDirection = DIR_LEFT;  break;
        case DIR_LEFT:  state->nextDirection = DIR_DOWN;  break;
        case DIR_DOWN:  state->nextDirection = DIR_RIGHT; break;
        case DIR_RIGHT: state->nextDirection = DIR_UP;    break;
    }
}

void Snake_TurnRight(GameState *state) {
    /* Maps relative 'Right' to cardinal directions based on current heading */
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
    
    /* Finalize the turn queued by the user */
    state->snake.direction = state->nextDirection;

    /* Calculate new head position */
    switch (state->snake.direction) {
        case DIR_UP:    newY--; break;
        case DIR_DOWN:  newY++; break;
        case DIR_LEFT:  newX--; break;
        case DIR_RIGHT: newX++; break;
    }

    /* Border collision check */
    if (newX < 0 || newX >= FIELD_W || newY < 0 || newY >= FIELD_H) {
        state->gameOver = true;
        return;
    }

    bool grow = ((uint8_t)newX == state->foodX && (uint8_t)newY == state->foodY);

    /* Self-collision check */
    for (uint8_t i = 0; i < state->snake.length; i++) {
        if (state->snake.x[i] == (uint8_t)newX && state->snake.y[i] == (uint8_t)newY) {
            /* Collision if hitting any part except the tail (which will move unless we grow) */
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

    /* Body segment shift: move each segment to the position of the one ahead */
    for (int16_t i = (int16_t)state->snake.length - 1; i > 0; i--) {
        state->snake.x[i] = state->snake.x[i - 1];
        state->snake.y[i] = state->snake.y[i - 1];
    }

    /* Update head */
    state->snake.x[0] = (uint8_t)newX;
    state->snake.y[0] = (uint8_t)newY;

    if (grow) PlaceFood(state);
    
    /* Clear turn queue lock for the next game step */
    state->turnQueued = false;
}
