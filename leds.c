#include "stm32f091xc.h"
#include "stdbool.h"
#include "leds.h"
#include "main.h"

// Functie om alle LED's op uitgang te zetten.
void InitLeds(void)
{
	// Clock voor GPIOA inschakelen.
	RCC->AHBENR = RCC->AHBENR | RCC_AHBENR_GPIOAEN;
	
	// Clock voor GPIOB inschakelen.
	RCC->AHBENR = RCC->AHBENR | RCC_AHBENR_GPIOBEN;
	
	// Clock voor GPIOC inschakelen.
	RCC->AHBENR = RCC->AHBENR | RCC_AHBENR_GPIOCEN;	
	
	// LED1 tot LED8 op output zetten.
	GPIOC->MODER = (GPIOC->MODER & ~GPIO_MODER_MODER0) | GPIO_MODER_MODER0_0;
	GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODER3) | GPIO_MODER_MODER3_0;
	GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODER5) | GPIO_MODER_MODER5_0;
	GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODER4) | GPIO_MODER_MODER4_0;
	GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODER10) | GPIO_MODER_MODER10_0;
	GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER8) | GPIO_MODER_MODER8_0;
	GPIOC->MODER = (GPIOC->MODER & ~GPIO_MODER_MODER7) | GPIO_MODER_MODER7_0;
	GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODER6) | GPIO_MODER_MODER6_0;
	
	// User LED op output zetten.
	GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER5) | GPIO_MODER_MODER5_0;
}

// Functie om één LED, in te schakelen.
void SetLed(uint8_t ledNumber)
{	
	switch(ledNumber)
	{
		// User LED.
		case 0:
			GPIOA->BSRR |= GPIO_BSRR_BS_5;
			break;
		
		// Nucleo Extension Shield LED's.
		case 1:
			GPIOC->BSRR |= GPIO_BSRR_BS_0;
			break;
		
		case 2:
			GPIOB->BSRR |= GPIO_BSRR_BS_3;
			break;
		
		case 3:
			GPIOB->BSRR |= GPIO_BSRR_BS_5;
			break;
		
		case 4:
			GPIOB->BSRR |= GPIO_BSRR_BS_4;
			break;
		
		case 5:
			GPIOB->BSRR |= GPIO_BSRR_BS_10;
			break;
		
		case 6:
			GPIOA->BSRR |= GPIO_BSRR_BS_8;
			break;
		
		case 7:
			GPIOC->BSRR |= GPIO_BSRR_BS_7;
			break;
		
		case 8:
			GPIOB->BSRR |= GPIO_BSRR_BS_6;
			break;
		
		default:
			break;
	}
}

// Functie om één LED, te doven.
void ClearLed(uint8_t ledNumber)
{	
	switch(ledNumber)
	{
		// User LED.
		case 0:
			GPIOA->BSRR |= GPIO_BSRR_BR_5;
			break;
		
		// Nucleo Extension Shield LED's.
		case 1:
			GPIOC->BSRR |= GPIO_BSRR_BR_0;
			break;
		
		case 2:
			GPIOB->BSRR |= GPIO_BSRR_BR_3;
			break;
		
		case 3:
			GPIOB->BSRR |= GPIO_BSRR_BR_5;
			break;
		
		case 4:
			GPIOB->BSRR |= GPIO_BSRR_BR_4;
			break;
		
		case 5:
			GPIOB->BSRR |= GPIO_BSRR_BR_10;
			break;
		
		case 6:
			GPIOA->BSRR |= GPIO_BSRR_BR_8;
			break;
		
		case 7:
			GPIOC->BSRR |= GPIO_BSRR_BR_7;
			break;
		
		case 8:
			GPIOB->BSRR |= GPIO_BSRR_BR_6;
			break;
		
		default:
			break;
	}
}

// Functie om één LED, om te keren van toestand.
void ToggleLed(uint8_t ledNumber)
{
	switch(ledNumber)
	{
		// User LED.
		case 0:
			GPIOA->ODR = GPIOA->ODR ^ GPIO_ODR_5;
			break;
		
		// Nucleo Extension Shield LED's.
		case 1:
			GPIOC->ODR = GPIOC->ODR ^ GPIO_ODR_0;
			break;
		
		case 2:
			GPIOB->ODR = GPIOB->ODR ^ GPIO_ODR_3;
			break;
		
		case 3:
			GPIOB->ODR = GPIOB->ODR ^ GPIO_ODR_5;
			break;
		
		case 4:
			GPIOB->ODR = GPIOB->ODR ^ GPIO_ODR_4;
			break;
		
		case 5:
			GPIOB->ODR = GPIOB->ODR ^ GPIO_ODR_10;
			break;
		
		case 6:
			GPIOA->ODR = GPIOA->ODR ^ GPIO_ODR_8;
			break;
		
		case 7:
			GPIOC->ODR = GPIOC->ODR ^ GPIO_ODR_7;
			break;
		
		case 8:
			GPIOB->ODR = GPIOB->ODR ^ GPIO_ODR_6;
			break;
		
		default:
			break;
	}
}

// Functie om één LED, even te laten knipperen.
// OPM: de flitstijd wordt hier met opzet beperkt tot 255 ms...
void FlashLed(uint8_t ledNumber, uint8_t flashTime)
{
	SetLed(ledNumber);
	WaitForMs(flashTime);
	ClearLed(ledNumber);	
}

// Functie om de data van één byte op de 8 LED's te plaatsen.
void ByteToLeds(uint8_t data)
{
	if(data & 0x01)
		GPIOC->ODR = GPIOC->ODR | GPIO_ODR_0;
	else
		GPIOC->ODR = GPIOC->ODR & ~GPIO_ODR_0;

	if(data & 0x02)
		GPIOB->ODR = GPIOB->ODR | GPIO_ODR_3;
	else
		GPIOB->ODR = GPIOB->ODR & ~GPIO_ODR_3;

	if(data & 0x04)
		GPIOB->ODR = GPIOB->ODR | GPIO_ODR_5;
	else
		GPIOB->ODR = GPIOB->ODR & ~GPIO_ODR_5;

	if(data & 0x08)
		GPIOB->ODR = GPIOB->ODR | GPIO_ODR_4;
	else
		GPIOB->ODR = GPIOB->ODR & ~GPIO_ODR_4;

	if(data & 0x10)
		GPIOB->ODR = GPIOB->ODR | GPIO_ODR_10;
	else
		GPIOB->ODR = GPIOB->ODR & ~GPIO_ODR_10;

	if(data & 0x20)
		GPIOA->ODR = GPIOA->ODR | GPIO_ODR_8;
	else
		GPIOA->ODR = GPIOA->ODR & ~GPIO_ODR_8;

	if(data & 0x40)
		GPIOC->ODR = GPIOC->ODR | GPIO_ODR_7;
	else
		GPIOC->ODR = GPIOC->ODR & ~GPIO_ODR_7;

	if(data & 0x80)
		GPIOB->ODR = GPIOB->ODR | GPIO_ODR_6;
	else
		GPIOB->ODR = GPIOB->ODR & ~GPIO_ODR_6;
}

// Functie om de data van één byte als 'niveau' op de 8 LED's te plaatsen.
void ByteToLevel(uint8_t level)
{
    if(level<16)
        ByteToLeds(0);
    else if(level < 48)
        ByteToLeds(1);
    else if(level < 80)
        ByteToLeds(3);
    else if(level < 112)
        ByteToLeds(7);
    else if(level < 144)
        ByteToLeds(15);
    else if(level < 176)
        ByteToLeds(0x1F);
    else if(level < 208)
        ByteToLeds(0x3F);
    else if(level < 240)
        ByteToLeds(0x7F);
    else
        ByteToLeds(255);
}
