#include "stm32f091xc.h"
#include "system_utils.h"

static volatile uint32_t ticks = 0;

void System_Init(void) {
    // Clock setup (48MHz from HSI)
    RCC->CR |= RCC_CR_HSITRIM_4;
    RCC->CR |= RCC_CR_HSION;
    while(!(RCC->CR & RCC_CR_HSIRDY));
    
    RCC->CFGR &= ~RCC_CFGR_SW;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
    
    RCC->CR &= ~RCC_CR_PLLON;
    while(RCC->CR & RCC_CR_PLLRDY);
    
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSI_PREDIV;
    RCC->CFGR2 |= RCC_CFGR2_PREDIV_DIV2;
    RCC->CFGR |= RCC_CFGR_PLLMUL12;
    
    FLASH->ACR |= FLASH_ACR_LATENCY;
    
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));

    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
    
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
    RCC->CFGR |= RCC_CFGR_PPRE_DIV1;
    
    SystemCoreClockUpdate();
    SysTick_Config(48000); // 1ms interrupt
}

void System_DelayMs(uint32_t ms) {
    uint32_t start = ticks;
    while((ticks - start) < ms);
}

uint32_t System_GetTicks(void) {
    return ticks;
}

void SysTick_Handler(void) {
    ticks++;
}
