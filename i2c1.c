#include "i2c1.h"

void I2C1_Init(void) {
    // Enable GPIOB clock
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    
    // Reset and un-reset I2C1
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;
    
    // Configure PB8 (SCL) and PB9 (SDA) as AF1
    GPIOB->MODER = (GPIOB->MODER & ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9)) | 
                   GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1;
    GPIOB->AFR[1] = (GPIOB->AFR[1] & ~0x000000FF) | 0x00000011;
    GPIOB->OTYPER |= GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9; // Open-drain
    
    // Select HSI as I2C1 clock (8MHz)
    RCC->CFGR3 &= ~RCC_CFGR3_I2C1SW;
    
    // Enable I2C1 clock
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    
    // Timing for 100kHz @ 8MHz HSI
    I2C1->TIMINGR = 0x00201D2B;
    
    // Enable I2C1
    I2C1->CR1 |= I2C_CR1_PE;
}

void I2C1_WriteArray(uint8_t deviceAddress, const uint8_t* data, uint8_t length) {
    while(I2C1->ISR & I2C_ISR_BUSY);

    // Clear CR2 then set parameters
    I2C1->CR2 = (uint32_t)((deviceAddress << 1) | (length << 16) | I2C_CR2_START);
    
    for(uint8_t i = 0; i < length; i++) {
        while(!(I2C1->ISR & I2C_ISR_TXE));
        I2C1->TXDR = data[i];
    }
    
    while(!(I2C1->ISR & I2C_ISR_TC));
    I2C1->CR2 |= I2C_CR2_STOP;
}

void I2C1_WriteRegisterByte(uint8_t deviceAddress, uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    I2C1_WriteArray(deviceAddress, data, 2);
}

uint8_t I2C1_ReadRegisterByte(uint8_t deviceAddress, uint8_t reg) {
    while(I2C1->ISR & I2C_ISR_BUSY);

    // Send register address
    I2C1->CR2 = (uint32_t)((deviceAddress << 1) | (1 << 16) | I2C_CR2_START);
    while(!(I2C1->ISR & I2C_ISR_TXE));
    I2C1->TXDR = reg;
    while(!(I2C1->ISR & I2C_ISR_TC));

    // Receive data (Restart)
    I2C1->CR2 = (uint32_t)((deviceAddress << 1) | (1 << 16) | I2C_CR2_RD_WRN | I2C_CR2_START);
    while(!(I2C1->ISR & I2C_ISR_RXNE));
    uint8_t value = (uint8_t)I2C1->RXDR;
    
    while(!(I2C1->ISR & I2C_ISR_TC));
    I2C1->CR2 |= I2C_CR2_STOP;
    
    return value;
}
