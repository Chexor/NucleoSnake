#include "buttons.h"

void Buttons_Init(void) {
    // Enable clocks for GPIOA, GPIOB, and GPIOC
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;
    
    // Set pins as input (MODER bits 00)
    GPIOA->MODER &= ~(GPIO_MODER_MODER1 | GPIO_MODER_MODER4);
    GPIOB->MODER &= ~GPIO_MODER_MODER0;
    GPIOC->MODER &= ~(GPIO_MODER_MODER1 | GPIO_MODER_MODER13);
}

bool SW1_IsActive(void) {
    return (GPIOA->IDR & GPIO_IDR_1) == 0;
}

bool SW2_IsActive(void) {
    return (GPIOA->IDR & GPIO_IDR_4) == 0;
}

bool SW3_IsActive(void) {
    return (GPIOB->IDR & GPIO_IDR_0) == 0;
}

bool SW4_IsActive(void) {
    return (GPIOC->IDR & GPIO_IDR_1) == 0;
}

bool UserButton_IsActive(void) {
    return (GPIOC->IDR & GPIO_IDR_13) == 0;
}
