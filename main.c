/**
 * @file main.c
 * @brief Application entry point and coordination of game flow.
 * 
 * This file handles the "Glue" logic:
 * - System initialization.
 * - Input polling and debounce.
 * - UI screens (Start, Game Over).
 * - Main loop timing (Fixed-step game loop).
 */

#include "stm32f091xc.h"
#include "stdbool.h"
#include "string.h"
#include "buttons.h"
#include "i2c1.h"
#include "oled.h"
#include "snake_engine.h"
#include "system_utils.h"

/* Layout Constants */
#define CELL_SIZE                   4U    /* Size of one snake cell in pixels */
#define FIELD_X_OFFSET              2U    /* Horizontal border padding */
#define FIELD_Y_OFFSET              2U    /* Vertical border padding */

/* Timing Constants */
#define GAME_STEP_MS                140U  /* Speed of the snake (lower = faster) */
#define TURN_DEBOUNCE_MS            120U  /* Min time between direction changes */
#define START_TONGUE_STEP_MS        600U  /* Tongue flick animation idle time */
#define START_TONGUE_FLICK_MS       120U  /* Tongue flick duration */

static uint8_t framebuffer[OLED_NUMBER_OF_PAGES][OLED_WIDTH];
static uint32_t lastTurnTick = 0;
static GameState game;

/**
 * @brief Clears the local 1KB framebuffer.
 */
static void ClearFrame(void) {
    memset(framebuffer, 0, sizeof(framebuffer));
}

/**
 * @brief Sets a single pixel in the framebuffer.
 */
static void SetPixel(uint8_t x, uint8_t y) {
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    /* framebuffer[page][column] */
    framebuffer[y >> 3][x] |= (uint8_t)(1U << (y & 0x07U));
}

/**
 * @brief Draws a 1-pixel wide border around the screen edge.
 */
static void DrawBorder(void) {
    for (uint8_t x = 0; x < OLED_WIDTH; x++) {
        SetPixel(x, 0);
        SetPixel(x, OLED_HEIGHT - 1U);
    }
    for (uint8_t y = 0; y < OLED_HEIGHT; y++) {
        SetPixel(0, y);
        SetPixel(OLED_WIDTH - 1U, y);
    }
}

/**
 * @brief Draws a 4x4 game cell (snake segment or food).
 * 
 * @param cellX Horizontal grid coordinate (0-30).
 * @param cellY Vertical grid coordinate (0-14).
 * @param filled If true, draws a solid block. If false, draws an outline (for food).
 */
static void DrawCell(uint8_t cellX, uint8_t cellY, bool filled) {
    uint8_t startX = (uint8_t)(FIELD_X_OFFSET + (cellX * CELL_SIZE));
    uint8_t startY = (uint8_t)(FIELD_Y_OFFSET + (cellY * CELL_SIZE));

    for (uint8_t y = 0; y < CELL_SIZE; y++) {
        for (uint8_t x = 0; x < CELL_SIZE; x++) {
            /* Pattern for empty cell (food): draws the inner cross */
            if (filled || (x == 1U) || (x == 2U) || (y == 1U) || (y == 2U))
                SetPixel((uint8_t)(startX + x), (uint8_t)(startY + y));
        }
    }
}

/**
 * @brief Flushes the entire framebuffer to the OLED.
 */
static void SendFrameToOled(void) {
    for (uint8_t page = 0; page < OLED_NUMBER_OF_PAGES; page++)
        OLED_ArrayToPage(framebuffer[page], OLED_WIDTH, page);
}

/**
 * @brief Orchestrates the rendering of the current game frame.
 */
static void RenderGame(void) {
    ClearFrame();
    DrawBorder();
    DrawCell(game.foodX, game.foodY, false);
    for (uint8_t i = 0; i < game.snake.length; i++)
        DrawCell(game.snake.x[i], game.snake.y[i], true);
    SendFrameToOled();
}

/**
 * @brief Renders the Game Over screen with score.
 */
static void ShowGameOver(void) {
    char scoreText[] = "    SCORE: 000";
    scoreText[11] = (char)('0' + ((game.score / 100U) % 10U));
    scoreText[12] = (char)('0' + ((game.score / 10U) % 10U));
    scoreText[13] = (char)('0' + (game.score % 10U));

    OLED_FillScreen(0x00);
    OLED_StringToPage(":::::::::::::::::::::", 1, true, true);
    OLED_StringToPage("      GAME OVER      ", 2, true, true);
    OLED_StringToPage(":::::::::::::::::::::", 3, true, true);
    OLED_StringToPage("", 4, true, false);
    OLED_StringToPage(scoreText, 5, true, false);
    OLED_StringToPage("    B1 to RESTART    ", 7, true, false);
}

/**
 * @brief Draws the snake head/tongue for the start screen animation.
 */
static void DrawStartTongue(bool tongueExtended) {
    OLED_StringToPage(tongueExtended ? "    SNAKE   \\  -----<" : "    SNAKE   \\  ---<", 2, true, false);
}

/**
 * @brief Renders the main menu/start screen.
 */
static void ShowStartScreen(void) {
    OLED_FillScreen(0x00);
    OLED_StringToPage("             ____", 0, true, false);
    OLED_StringToPage("   NUCLEO   / . .\\", 1, true, false);
    DrawStartTongue(false);
    OLED_StringToPage("             \\  /", 3, true, false);
    OLED_StringToPage("   __________/ /", 4, true, false);
    OLED_StringToPage("-=:___________/", 5, true, false);
    OLED_StringToPage("", 6, true, false);
    OLED_StringToPage("SW1:L SW4:R  B1:Start", 7, true, false);
}

/**
 * @brief Blocking wait for the start button with animation processing.
 */
static void WaitForStartButton(void) {
    uint32_t lastTongueTick = System_GetTicks();
    uint16_t tongueDelay = START_TONGUE_STEP_MS;
    uint8_t tongueStep = 0;

    while (!UserButton_IsActive()) {
        if ((System_GetTicks() - lastTongueTick) >= tongueDelay) {
            lastTongueTick = System_GetTicks();
            /* Animated sequence for the flicking tongue */
            if (tongueStep == 0U) { DrawStartTongue(true); tongueDelay = START_TONGUE_FLICK_MS; tongueStep = 1U; }
            else if (tongueStep == 1U) { DrawStartTongue(false); tongueDelay = START_TONGUE_FLICK_MS; tongueStep = 2U; }
            else if (tongueStep == 2U) { DrawStartTongue(true); tongueDelay = START_TONGUE_FLICK_MS; tongueStep = 3U; }
            else { DrawStartTongue(false); tongueDelay = START_TONGUE_STEP_MS; tongueStep = 0U; }
        }
    }
    System_DelayMs(200); /* Simple debounce */
    while (UserButton_IsActive()); /* Wait for release */
}

/**
 * @brief Reads user input and updates the queued direction.
 * Uses a debounce timer to prevent "double turns" from switch bounce.
 */
static void ReadInput(void) {
    static bool sw1WasActive = false;
    static bool sw4WasActive = false;
    bool sw1IsActive = SW1_IsActive();
    bool sw4IsActive = SW4_IsActive();
    uint32_t ticks = System_GetTicks();

    /* Input lock: only one turn allowed per snake movement step */
    if (game.turnQueued || ((ticks - lastTurnTick) < TURN_DEBOUNCE_MS)) {
        sw1WasActive = sw1IsActive;
        sw4WasActive = sw4IsActive;
        return;
    }

    if (sw1IsActive && !sw1WasActive) {
        Snake_TurnLeft(&game);
        game.turnQueued = true;
        lastTurnTick = ticks;
    } else if (sw4IsActive && !sw4WasActive) {
        Snake_TurnRight(&game);
        game.turnQueued = true;
        lastTurnTick = ticks;
    }
    sw1WasActive = sw1IsActive;
    sw4WasActive = sw4IsActive;
}

/**
 * @brief Application Main Loop.
 */
int main(void) {
    uint32_t lastStepTick = 0;
    bool gameOverShown = false;

    /* 1. Hardware Initialization */
    System_Init();
    Buttons_Init();
    I2C1_Init();
    System_DelayMs(100);
    OLED_Init();

    /* 2. Menu Phase */
    ShowStartScreen();
    WaitForStartButton();
    
    /* 3. Game Phase */
    Snake_Reset(&game, System_GetTicks());
    RenderGame();
    lastStepTick = System_GetTicks();

    while (1) {
        if (game.gameOver) {
            /* Handle Game Over state */
            if (!gameOverShown) { 
                ShowGameOver(); 
                gameOverShown = true; 
            }
            if (UserButton_IsActive()) {
                System_DelayMs(200);
                Snake_Reset(&game, System_GetTicks());
                RenderGame();
                lastStepTick = System_GetTicks();
                gameOverShown = false;
            }
        } else {
            /* Handle Active Game state */
            ReadInput();
            
            /* Fixed-step movement timing */
            if ((System_GetTicks() - lastStepTick) >= GAME_STEP_MS) {
                lastStepTick = System_GetTicks();
                Snake_Move(&game);
                RenderGame();
            }
        }
    }
}
