#include "stm32f091xc.h"
#include "stdbool.h"
#include "string.h"
#include "buttons.h"
#include "i2c1.h"
#include "oled.h"
#include "snake_engine.h"
#include "system_utils.h"

#define CELL_SIZE                   4U
#define FIELD_X_OFFSET              2U
#define FIELD_Y_OFFSET              2U
#define GAME_STEP_MS                140U
#define TURN_DEBOUNCE_MS            120U
#define START_TONGUE_STEP_MS        600U
#define START_TONGUE_FLICK_MS       120U

static uint8_t framebuffer[OLED_NUMBER_OF_PAGES][OLED_WIDTH];
static uint32_t lastTurnTick = 0;
static GameState game;

static void ClearFrame(void) {
    memset(framebuffer, 0, sizeof(framebuffer));
}

static void SetPixel(uint8_t x, uint8_t y) {
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    framebuffer[y >> 3][x] |= (uint8_t)(1U << (y & 0x07U));
}

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

static void DrawCell(uint8_t cellX, uint8_t cellY, bool filled) {
    uint8_t startX = (uint8_t)(FIELD_X_OFFSET + (cellX * CELL_SIZE));
    uint8_t startY = (uint8_t)(FIELD_Y_OFFSET + (cellY * CELL_SIZE));

    for (uint8_t y = 0; y < CELL_SIZE; y++) {
        for (uint8_t x = 0; x < CELL_SIZE; x++) {
            if (filled || (x == 1U) || (x == 2U) || (y == 1U) || (y == 2U))
                SetPixel((uint8_t)(startX + x), (uint8_t)(startY + y));
        }
    }
}

static void SendFrameToOled(void) {
    for (uint8_t page = 0; page < OLED_NUMBER_OF_PAGES; page++)
        OLED_ArrayToPage(framebuffer[page], OLED_WIDTH, page);
}

static void RenderGame(void) {
    ClearFrame();
    DrawBorder();
    DrawCell(game.foodX, game.foodY, false);
    for (uint8_t i = 0; i < game.snake.length; i++)
        DrawCell(game.snake.x[i], game.snake.y[i], true);
    SendFrameToOled();
}

static void ShowGameOver(void) {
    char scoreText[] = "    SCORE: 000";
    scoreText[11] = (char)('0' + ((game.score / 100U) % 10U));
    scoreText[12] = (char)('0' + ((game.score / 10U) % 10U));
    scoreText[13] = (char)('0' + (game.score % 10U));

    OLED_FillScreen(0x00);
    OLED_StringToPage(":::::::::::::::::::::", 1, true, true);
    OLED_StringToPage("      YOU DIED      ", 2, true, true);
    OLED_StringToPage(":::::::::::::::::::::", 3, true, true);
		OLED_StringToPage("",4, true, false);
    OLED_StringToPage(scoreText, 5, true, false);
    OLED_StringToPage("    B1 to RESTART    ", 7, true, false);
}

static void DrawStartTongue(bool tongueExtended) {
    OLED_StringToPage(tongueExtended ? "    SNAKE   \\  -----<" : "    SNAKE   \\  ---<", 2, true, false);
}

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

static void WaitForStartButton(void) {
    uint32_t lastTongueTick = System_GetTicks();
    uint16_t tongueDelay = START_TONGUE_STEP_MS;
    uint8_t tongueStep = 0;

    while (!UserButton_IsActive()) {
        if ((System_GetTicks() - lastTongueTick) >= tongueDelay) {
            lastTongueTick = System_GetTicks();
            if (tongueStep == 0U) { DrawStartTongue(true); tongueDelay = START_TONGUE_FLICK_MS; tongueStep = 1U; }
            else if (tongueStep == 1U) { DrawStartTongue(false); tongueDelay = START_TONGUE_FLICK_MS; tongueStep = 2U; }
            else if (tongueStep == 2U) { DrawStartTongue(true); tongueDelay = START_TONGUE_FLICK_MS; tongueStep = 3U; }
            else { DrawStartTongue(false); tongueDelay = START_TONGUE_STEP_MS; tongueStep = 0U; }
        }
    }
    System_DelayMs(200);
    while (UserButton_IsActive());
}

static void ReadInput(void) {
    static bool sw1WasActive = false;
    static bool sw4WasActive = false;
    bool sw1IsActive = SW1_IsActive();
    bool sw4IsActive = SW4_IsActive();
    uint32_t ticks = System_GetTicks();

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

int main(void) {
    uint32_t lastStepTick = 0;
    bool gameOverShown = false;

    System_Init();
    Buttons_Init();
    I2C1_Init();
    System_DelayMs(100);
    OLED_Init();
    ShowStartScreen();
    WaitForStartButton();
    
    Snake_Reset(&game, System_GetTicks());
    RenderGame();
    lastStepTick = System_GetTicks();

    while (1) {
        if (game.gameOver) {
            if (!gameOverShown) { ShowGameOver(); gameOverShown = true; }
            if (UserButton_IsActive()) {
                System_DelayMs(200);
                Snake_Reset(&game, System_GetTicks());
                RenderGame();
                lastStepTick = System_GetTicks();
                gameOverShown = false;
            }
        } else {
            ReadInput();
            if ((System_GetTicks() - lastStepTick) >= GAME_STEP_MS) {
                lastStepTick = System_GetTicks();
                Snake_Move(&game);
                RenderGame();
            }
        }
    }
}
