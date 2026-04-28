/**
 * @file buttons.c
 * @brief GPIO polling driver for the Nucleo expansion shield buttons.
 */

#include "buttons.h"

/**
 * @brief Configures GPIO pins for SW1-SW4 and the Blue User Button.
 */
void Buttons_Init(void) {
    /* Enable clocks for GPIO ports A, B, and C */
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;
    
    /* MODER bits: 00 = Input mode (Default after reset)
     * PA1 (SW1), PA4 (SW2), PB0 (SW3), PC1 (SW4), PC13 (B1)
     */
    GPIOA->MODER &= ~(GPIO_MODER_MODER1 | GPIO_MODER_MODER4);
    GPIOB->MODER &= ~GPIO_MODER_MODER0;
    GPIOC->MODER &= ~(GPIO_MODER_MODER1 | GPIO_MODER_MODER13);
}

/**
 * @brief All buttons are active low (grounded when pressed).
 * @return true if button is pressed.
 */

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
