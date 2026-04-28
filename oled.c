#include "oled.h"
#include "i2c1.h"
#include "font6x8.h"
#include <string.h>

#define OLED_COMMAND_CONTROL_BYTE   0x00
#define OLED_DATA_CONTROL_BYTE      0x40
#undef START_CHARACTER
#define START_CHARACTER             0x00

static void OLED_SendCommand(uint8_t command) {
    I2C1_WriteRegisterByte(OLED_SSD1306_ADDRESS, OLED_COMMAND_CONTROL_BYTE, command);
}

static void OLED_SetPageAddress(uint8_t page) {
    OLED_SendCommand(0xB0 | (page & 0x07));
}

static void OLED_SetColumnAddress(uint8_t col) {
    OLED_SendCommand(0x00 | (col & 0x0F));
    OLED_SendCommand(0x10 | ((col & 0xF0) >> 4));
}

void OLED_Init(void) {
    const uint8_t init_cmds[] = {
        0xAE, // Display OFF
        0xA8, 0x3F, // Set MUX Ratio
        0xD3, 0x00, // Set Display Offset
        0x40, // Set Display Start Line
        0x20, 0x02, // Set Memory Addressing Mode (Page)
        0xA1, // Set Segment Re-map
        0xC8, // Set COM Output Scan Direction
        0xDA, 0x12, // Set COM Pins Hardware Config
        0x81, 0x7F, // Set Contrast
        0xA4, // Entire Display ON (Resume)
        0xA6, // Set Normal Display
        0xD5, 0x80, // Set OSC Frequency
        0x8D, 0x14, // Enable Charge Pump
        0xAF  // Display ON
    };

    for (uint8_t i = 0; i < sizeof(init_cmds); i++) {
        OLED_SendCommand(init_cmds[i]);
    }
    OLED_FillScreen(0x00);
}

void OLED_FillPage(uint8_t data, uint8_t page) {
    uint8_t i2cData[OLED_WIDTH + 1];
    i2cData[0] = OLED_DATA_CONTROL_BYTE;
    memset(&i2cData[1], data, OLED_WIDTH);

    OLED_SetPageAddress(page);
    OLED_SetColumnAddress(0);
    I2C1_WriteArray(OLED_SSD1306_ADDRESS, i2cData, OLED_WIDTH + 1);
}

void OLED_FillScreen(uint8_t data) {
    for (uint8_t i = 0; i < OLED_NUMBER_OF_PAGES; i++) {
        OLED_FillPage(data, i);
    }
}

void OLED_ArrayToPage(const uint8_t* data, uint8_t length, uint8_t page) {
    if (length > OLED_WIDTH) length = OLED_WIDTH;
    
    uint8_t i2cData[OLED_WIDTH + 1];
    i2cData[0] = OLED_DATA_CONTROL_BYTE;
    memcpy(&i2cData[1], data, length);

    OLED_SetPageAddress(page);
    OLED_SetColumnAddress(0);
    I2C1_WriteArray(OLED_SSD1306_ADDRESS, i2cData, length + 1);
}

void OLED_StringToPage(const char* text, uint8_t page, bool fillWithBlanks, bool invert) {
    uint8_t i2cData[OLED_WIDTH + 1];
    i2cData[0] = OLED_DATA_CONTROL_BYTE;
    uint8_t col = 0;
    uint8_t textLen = strlen(text);
    uint8_t stride = CHARACTER_WIDTH + 1;

    memset(&i2cData[1], invert ? 0xFF : 0x00, OLED_WIDTH);

    for (uint8_t i = 0; i < textLen && (col + stride) <= OLED_WIDTH; i++) {
        if (text[i] == '\r' || text[i] == '\n') break;
        
        uint32_t charIdx = (uint32_t)text[i] - START_CHARACTER;
        uint8_t charData[CHARACTER_WIDTH];
        memcpy(charData, &font6x8[charIdx * CHARACTER_WIDTH], CHARACTER_WIDTH);
        
        if (invert) {
            for (uint8_t b = 0; b < CHARACTER_WIDTH; b++) charData[b] = ~charData[b];
        }
        
        memcpy(&i2cData[1 + col], charData, CHARACTER_WIDTH);
        col += stride;
    }

    if (fillWithBlanks) {
        OLED_SetPageAddress(page);
        OLED_SetColumnAddress(0);
        I2C1_WriteArray(OLED_SSD1306_ADDRESS, i2cData, OLED_WIDTH + 1);
    } else {
        OLED_SetPageAddress(page);
        OLED_SetColumnAddress(0);
        I2C1_WriteArray(OLED_SSD1306_ADDRESS, i2cData, col + 1);
    }
}
