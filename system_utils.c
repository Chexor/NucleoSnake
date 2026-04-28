/**
 * @file system_utils.c
 * @brief STM32F091 Clock configuration and timing utilities.
 * 
 * CONCEPT: The STM32 clock tree is complex. This module configures the 
 * Internal High Speed (HSI) oscillator (8MHz) and uses a Phase-Locked 
 * Loop (PLL) to multiply it to 48MHz for maximum performance.
 */

#include "stm32f091xc.h"
#include "system_utils.h"

static volatile uint32_t ticks = 0;

/**
 * @brief Initializes the System Clock and SysTick Timer.
 * 
 * CONFIGURATION STEPS:
 * 1. Enable HSI (8MHz).
 * 2. Configure Flash Latency (required for high speeds > 24MHz).
 * 3. Set PLL Source to HSI / 2 (4MHz).
 * 4. Multiply PLL by 12 (4MHz * 12 = 48MHz).
 * 5. Select PLL as System Clock source.
 * 6. Configure SysTick to generate an interrupt every 48,000 cycles (1ms).
 */
void System_Init(void) {
    /* HSITRIM at default (16) */
    RCC->CR |= RCC_CR_HSITRIM_4;
    RCC->CR |= RCC_CR_HSION;
    while(!(RCC->CR & RCC_CR_HSIRDY));
    
    /* Ensure System clock is on HSI before changing PLL */
    RCC->CFGR &= ~RCC_CFGR_SW;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
    
    /* Disable PLL for configuration */
    RCC->CR &= ~RCC_CR_PLLON;
    while(RCC->CR & RCC_CR_PLLRDY);
    
    /* PLL input: HSI / 2 = 4MHz */
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSI_PREDIV;
    RCC->CFGR2 |= RCC_CFGR2_PREDIV_DIV2;
    
    /* PLL multiplier: 12x = 48MHz */
    RCC->CFGR |= RCC_CFGR_PLLMUL12;
    
    /* Set 1 Wait State for Flash (required for 48MHz) */
    FLASH->ACR |= FLASH_ACR_LATENCY;
    
    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));

    /* Select PLL as System Clock */
    RCC->CFGR |= RCC_CR_PLLON; /* Set SW bits to PLL */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
    
    /* Set Bus Prescalers (HCLK and PCLK = 48MHz) */
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
    RCC->CFGR |= RCC_CFGR_PPRE_DIV1;
    
    SystemCoreClockUpdate();
    SysTick_Config(48000); /* Interrupt every 1ms at 48MHz */
}

void System_DelayMs(uint32_t ms) {
    uint32_t start = ticks;
    /* Uses subtraction to handle uint32_t rollover correctly */
    while((ticks - start) < ms);
}

uint32_t System_GetTicks(void) {
    return ticks;
}

/**
 * @brief SysTick Interrupt Handler. Increments the global tick counter.
 */
void SysTick_Handler(void) {
    ticks++;
}
