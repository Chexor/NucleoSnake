// Snake game voor Nucleo-F091RC + SSD1306 OLED.

#include "stm32f091xc.h"
#include "stdbool.h"
#include "string.h"
#include "buttons.h"
#include "i2c1.h"
#include "oled.h"
#include "main.h"

#define CELL_SIZE                   4U
#define FIELD_X_OFFSET              2U
#define FIELD_Y_OFFSET              2U
#define FIELD_W                     ((OLED_WIDTH - (2U * FIELD_X_OFFSET)) / CELL_SIZE)
#define FIELD_H                     ((OLED_HEIGHT - (2U * FIELD_Y_OFFSET)) / CELL_SIZE)
#define MAX_SNAKE_LENGTH            160U
#define GAME_STEP_MS                140U

typedef enum
{
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

static volatile uint32_t ticks = 0;
static uint8_t framebuffer[OLED_NUMBER_OF_PAGES][OLED_WIDTH];

static uint8_t snakeX[MAX_SNAKE_LENGTH];
static uint8_t snakeY[MAX_SNAKE_LENGTH];
static uint8_t snakeLength = 0;
static uint8_t foodX = 0;
static uint8_t foodY = 0;
static Direction direction = DIR_RIGHT;
static Direction nextDirection = DIR_RIGHT;
static bool gameOver = false;
static uint16_t randomState = 0xACE1U;

static void ClearFrame(void)
{
    memset(framebuffer, 0, sizeof(framebuffer));
}

static void SetPixel(uint8_t x, uint8_t y)
{
    uint8_t page = y >> 3;
    uint8_t bit = y & 0x07U;

    framebuffer[page][x] |= (uint8_t)(1U << bit);
}

static void DrawBorder(void)
{
    uint8_t x = 0;
    uint8_t y = 0;

    for(x = 0; x < OLED_WIDTH; x++)
    {
        SetPixel(x, 0);
        SetPixel(x, OLED_HEIGHT - 1U);
    }

    for(y = 0; y < OLED_HEIGHT; y++)
    {
        SetPixel(0, y);
        SetPixel(OLED_WIDTH - 1U, y);
    }
}

static void DrawCell(uint8_t cellX, uint8_t cellY, bool filled)
{
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t startX = (uint8_t)(FIELD_X_OFFSET + (cellX * CELL_SIZE));
    uint8_t startY = (uint8_t)(FIELD_Y_OFFSET + (cellY * CELL_SIZE));

    for(y = 0; y < CELL_SIZE; y++)
    {
        for(x = 0; x < CELL_SIZE; x++)
        {
            if(filled || (x == 1U) || (x == 2U) || (y == 1U) || (y == 2U))
                SetPixel((uint8_t)(startX + x), (uint8_t)(startY + y));
        }
    }
}

static void SendFrameToOled(void)
{
    uint8_t page = 0;

    for(page = 0; page < OLED_NUMBER_OF_PAGES; page++)
        OLED_ArrayToPage(framebuffer[page], OLED_WIDTH, page);
}

static uint16_t NextRandom(void)
{
    uint16_t bit = 0;

    bit = (uint16_t)(((randomState >> 0) ^ (randomState >> 2) ^ (randomState >> 3) ^ (randomState >> 5)) & 1U);
    randomState = (uint16_t)((randomState >> 1) | (bit << 15));

    return randomState;
}

static bool SnakeContains(uint8_t x, uint8_t y)
{
    uint8_t index = 0;

    for(index = 0; index < snakeLength; index++)
    {
        if((snakeX[index] == x) && (snakeY[index] == y))
            return true;
    }

    return false;
}

static void PlaceFood(void)
{
    uint16_t attempt = 0;

    for(attempt = 0; attempt < 512U; attempt++)
    {
        foodX = (uint8_t)(NextRandom() % FIELD_W);
        foodY = (uint8_t)(NextRandom() % FIELD_H);

        if(!SnakeContains(foodX, foodY))
            return;
    }

    foodX = 0;
    foodY = 0;
}

static void ResetGame(void)
{
    snakeLength = 5;
    snakeX[0] = 16;
    snakeY[0] = 8;
    snakeX[1] = 15;
    snakeY[1] = 8;
    snakeX[2] = 14;
    snakeY[2] = 8;
    snakeX[3] = 13;
    snakeY[3] = 8;
    snakeX[4] = 12;
    snakeY[4] = 8;
    direction = DIR_RIGHT;
    nextDirection = DIR_RIGHT;
    gameOver = false;
    randomState = (uint16_t)(ticks ^ 0xACE1U);
    PlaceFood();
}

static Direction TurnLeft(Direction dir)
{
    if(dir == DIR_UP)
        return DIR_LEFT;
    if(dir == DIR_LEFT)
        return DIR_DOWN;
    if(dir == DIR_DOWN)
        return DIR_RIGHT;

    return DIR_UP;
}

static Direction TurnRight(Direction dir)
{
    if(dir == DIR_UP)
        return DIR_RIGHT;
    if(dir == DIR_RIGHT)
        return DIR_DOWN;
    if(dir == DIR_DOWN)
        return DIR_LEFT;

    return DIR_UP;
}

static void ReadInput(void)
{
    static bool sw1WasActive = false;
    static bool sw4WasActive = false;
    bool sw1IsActive = SW1Active();
    bool sw4IsActive = SW4Active();

    if(sw1IsActive && !sw1WasActive)
        nextDirection = TurnLeft(nextDirection);
    else if(sw4IsActive && !sw4WasActive)
        nextDirection = TurnRight(nextDirection);

    sw1WasActive = sw1IsActive;
    sw4WasActive = sw4IsActive;
}

static void MoveSnake(void)
{
    int8_t newX = (int8_t)snakeX[0];
    int8_t newY = (int8_t)snakeY[0];
    int16_t index = 0;
    bool grow = false;

    direction = nextDirection;

    if(direction == DIR_UP)
        newY--;
    else if(direction == DIR_DOWN)
        newY++;
    else if(direction == DIR_LEFT)
        newX--;
    else
        newX++;

    if((newX < 0) || (newX >= FIELD_W) || (newY < 0) || (newY >= FIELD_H))
    {
        gameOver = true;
        return;
    }

    grow = ((uint8_t)newX == foodX) && ((uint8_t)newY == foodY);

    for(index = 0; index < snakeLength; index++)
    {
        if((snakeX[index] == (uint8_t)newX) && (snakeY[index] == (uint8_t)newY))
        {
            if(grow || (index != (snakeLength - 1)))
            {
                gameOver = true;
                return;
            }
        }
    }

    if(grow && (snakeLength < MAX_SNAKE_LENGTH))
        snakeLength++;

    for(index = (int16_t)snakeLength - 1; index > 0; index--)
    {
        snakeX[index] = snakeX[index - 1];
        snakeY[index] = snakeY[index - 1];
    }

    snakeX[0] = (uint8_t)newX;
    snakeY[0] = (uint8_t)newY;

    if(grow)
        PlaceFood();
}

static void RenderGame(void)
{
    uint8_t index = 0;

    ClearFrame();
    DrawBorder();
    DrawCell(foodX, foodY, false);

    for(index = 0; index < snakeLength; index++)
        DrawCell(snakeX[index], snakeY[index], true);

    SendFrameToOled();
}

static void ShowGameOver(void)
{
    OLED_FillScreen(0x00);
    OLED_StringToPage("      SNAKE", 2, true);
    OLED_StringToPage("    GAME OVER", 3, true);
    OLED_StringToPage("  B1 = opnieuw", 5, true);
}

int main(void)
{
    uint32_t lastStepTick = 0;
    bool gameOverShown = false;

    SystemClock_Config();
    InitButtons();
    InitI2C1();
    WaitForMs(100);
    OLED_Init();
    ResetGame();
    RenderGame();
    lastStepTick = ticks;

    while (1)
    {
        if(gameOver)
        {
            if(!gameOverShown)
            {
                ShowGameOver();
                gameOverShown = true;
            }

            if(UserButtonActive())
            {
                WaitForMs(200);
                ResetGame();
                RenderGame();
                lastStepTick = ticks;
                gameOverShown = false;
            }
        }
        else
        {
            ReadInput();

            if((ticks - lastStepTick) >= GAME_STEP_MS)
            {
                lastStepTick = ticks;
                MoveSnake();
                RenderGame();
            }
        }
    }
}

// Handler die iedere 1ms afloopt. Ingesteld met SystemCoreClockUpdate() en SysTick_Config().
void SysTick_Handler(void)
{
	ticks++;
}

// Wachtfunctie via de SysTick.
void WaitForMs(uint32_t timespan)
{
	uint32_t startTime = ticks;

	while(ticks < startTime + timespan)
	{
	}
}

// Klokken instellen. Deze functie niet wijzigen, tenzij je goed weet wat je doet.
void SystemClock_Config(void)
{
	RCC->CR |= RCC_CR_HSITRIM_4;													// HSITRIM op 16 zetten, dit is standaard (ook na reset).
	RCC->CR  |= RCC_CR_HSION;														// Internal high speed oscillator enable (8MHz)
	while((RCC->CR & RCC_CR_HSIRDY) == 0);									// Wacht tot HSI zeker ingeschakeld is
	
	RCC->CFGR &= ~RCC_CFGR_SW;													// System clock op HSI zetten (SWS is status geupdatet door hardware)	
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);	// Wachten to effectief HSI in actie is getreden
	
	RCC->CR &= ~RCC_CR_PLLON;													// Eerst PLL uitschakelen
	while((RCC->CR & RCC_CR_PLLRDY) != 0);								// Wacht tot PLL zeker uitgeschakeld is
	
	RCC->CFGR |= RCC_CFGR_PLLSRC_HSI_PREDIV;							// 01: HSI/PREDIV selected as PLL input clock
	RCC->CFGR2 |= RCC_CFGR2_PREDIV_DIV2;								// prediv = /2		=> 4MHz
	RCC->CFGR |= RCC_CFGR_PLLMUL12;										// PLL multiplied by 12 => 48MHz
	
	FLASH->ACR |= FLASH_ACR_LATENCY;										//  meer dan 24 MHz, dus latency op 1 (p 67)
	
	RCC->CR |= RCC_CR_PLLON;													// PLL inschakelen
	while((RCC->CR & RCC_CR_PLLRDY) == 0);								// Wacht tot PLL zeker ingeschakeld is

	RCC->CFGR |= RCC_CFGR_SW_PLL; 											// PLLCLK selecteren als SYSCLK (48MHz)
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);	// Wait until the PLL is switched on
		
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;										// SYSCLK niet meer delen, dus HCLK = 48MHz
	RCC->CFGR |= RCC_CFGR_PPRE_DIV1;										// HCLK niet meer delen, dus PCLK = 48MHz	
	
	SystemCoreClockUpdate();														// Nieuwe waarde van de core frequentie opslaan in SystemCoreClock variabele
	SysTick_Config(48000);														// Interrupt genereren. Zie core_cm0.h, om na ieder 1ms een interrupt
}
