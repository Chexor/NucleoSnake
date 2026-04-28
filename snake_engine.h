/**
 * @file snake_engine.h
 * @brief Logic and state management for the Snake game.
 * 
 * This module handles the "brain" of the game, including movement, 
 * collision detection, and score tracking. It is designed to be 
 * independent of the hardware (OLED, buttons, etc.).
 */

#ifndef SNAKE_ENGINE_H
#define SNAKE_ENGINE_H

#include <stdint.h>
#include <stdbool.h>

/* Game constants */
#define MAX_SNAKE_LENGTH    160U
#define FIELD_W             31U  /* (128px - 4px border) / 4px cells */
#define FIELD_H             15U  /* (64px - 4px border) / 4px cells */

/**
 * @brief Directional enumeration for snake movement.
 */
typedef enum {
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

/**
 * @brief Structure representing the snake's body and current heading.
 */
typedef struct {
    uint8_t x[MAX_SNAKE_LENGTH]; /* Body segments X coordinates */
    uint8_t y[MAX_SNAKE_LENGTH]; /* Body segments Y coordinates */
    uint8_t length;              /* Current number of segments */
    Direction direction;         /* Current movement direction */
} Snake;

/**
 * @brief Main Game State structure encapsulating all logic-related variables.
 */
typedef struct {
    Snake snake;
    uint8_t foodX;
    uint8_t foodY;
    uint16_t score;
    Direction nextDirection;     /* Queued direction for the next step */
    bool turnQueued;             /* Flag to prevent multiple turns per step */
    bool gameOver;
    uint16_t randomState;        /* Internal state for the LFSR generator */
} GameState;

/**
 * @brief Resets the game to initial parameters.
 * @param state Pointer to the game state.
 * @param seed Seed for the random number generator.
 */
void Snake_Reset(GameState *state, uint32_t seed);

/**
 * @brief Advances the snake by one step and handles collisions/food.
 * @param state Pointer to the game state.
 */
void Snake_Move(GameState *state);

/**
 * @brief Queues a relative 90-degree left turn.
 * @param state Pointer to the game state.
 */
void Snake_TurnLeft(GameState *state);

/**
 * @brief Queues a relative 90-degree right turn.
 * @param state Pointer to the game state.
 */
void Snake_TurnRight(GameState *state);

#endif // SNAKE_ENGINE_H
