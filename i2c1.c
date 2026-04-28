/**
 * @file i2c1.c
 * @brief Register-level I2C driver for STM32F091.
 * 
 * CONCEPT: I2C (Inter-Integrated Circuit) is a synchronous, multi-master 
 * bus. This implementation uses polling (blocking) for simplicity in this 
 * project. It directly manipulates the I2C control and status registers.
 */

#include "i2c1.h"

/**
 * @brief Configures I2C1 for 100kHz Standard Mode.
 */
void I2C1_Init(void) {
    /* Enable GPIOB clock */
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    
    /* Reset and un-reset I2C1 to ensure a clean state */
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;
    
    /* PB8 (SCL) and PB9 (SDA) as Alternate Function 1 (AF1) */
    GPIOB->MODER = (GPIOB->MODER & ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9)) | 
                   GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1;
    GPIOB->AFR[1] = (GPIOB->AFR[1] & ~0x000000FF) | 0x00000011;
    GPIOB->OTYPER |= GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9; /* Open-drain outputs */
    
    /* Select HSI (8MHz) as I2C1 clock source */
    RCC->CFGR3 &= ~RCC_CFGR3_I2C1SW;
    
    /* Enable I2C1 peripheral clock */
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    
    /* Timing configuration: 100kHz @ 8MHz HSI 
     * Derived from ST's STM32CubeIDE timing tool.
     */
    I2C1->TIMINGR = 0x00201D2B;
    
    /* Enable I2C1 peripheral */
    I2C1->CR1 |= I2C_CR1_PE;
}

/**
 * @brief Writes an array of bytes to a slave device.
 */
void I2C1_WriteArray(uint8_t deviceAddress, const uint8_t* data, uint8_t length) {
    /* Wait if the bus is busy from a previous transaction */
    while(I2C1->ISR & I2C_ISR_BUSY);

    /* Configure transfer: Slave Address, Length, and generate START condition */
    I2C1->CR2 = (uint32_t)((deviceAddress << 1) | (length << 16) | I2C_CR2_START);
    
    for(uint8_t i = 0; i < length; i++) {
        /* Wait until TX buffer is empty */
        while(!(I2C1->ISR & I2C_ISR_TXE));
        I2C1->TXDR = data[i];
    }
    
    /* Wait for Transmission Complete (TC) */
    while(!(I2C1->ISR & I2C_ISR_TC));
    
    /* Generate STOP condition */
    I2C1->CR2 |= I2C_CR2_STOP;
}

void I2C1_WriteRegisterByte(uint8_t deviceAddress, uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    I2C1_WriteArray(deviceAddress, data, 2);
}

/**
 * @brief Reads a register value from a slave device using a Restart condition.
 */
uint8_t I2C1_ReadRegisterByte(uint8_t deviceAddress, uint8_t reg) {
    while(I2C1->ISR & I2C_ISR_BUSY);

    /* Step 1: Send the register address we want to read */
    I2C1->CR2 = (uint32_t)((deviceAddress << 1) | (1 << 16) | I2C_CR2_START);
    while(!(I2C1->ISR & I2C_ISR_TXE));
    I2C1->TXDR = reg;
    while(!(I2C1->ISR & I2C_ISR_TC));

    /* Step 2: Send a RESTART + Read bit to receive data */
    I2C1->CR2 = (uint32_t)((deviceAddress << 1) | (1 << 16) | I2C_CR2_RD_WRN | I2C_CR2_START);
    
    /* Wait for Receive buffer Not Empty (RXNE) */
    while(!(I2C1->ISR & I2C_ISR_RXNE));
    uint8_t value = (uint8_t)I2C1->RXDR;
    
    while(!(I2C1->ISR & I2C_ISR_TC));
    I2C1->CR2 |= I2C_CR2_STOP;
    
    return value;
}
