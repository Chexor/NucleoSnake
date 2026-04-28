/**
 * @file oled.c
 * @brief Driver for the SSD1306 OLED display via I2C.
 * 
 * CONCEPT: Framebuffer Rendering. 
 * Instead of drawing directly to the OLED (which is slow and complex for 
 * individual pixels), we keep a 1KB "mirror" (framebuffer) in MCU RAM. 
 * We modify this buffer and then flush it to the OLED one page at a time.
 * 
 * SCREEN STRUCTURE:
 * 128 Columns x 64 Rows.
 * Organized into 8 "Pages" (horizontal bands of 8 pixels high).
 * Each byte in a page represents 8 vertical pixels (LSB at top).
 */

#include "oled.h"
#include "i2c1.h"
#include "font6x8.h"
#include <string.h>

#define OLED_COMMAND_CONTROL_BYTE   0x00
#define OLED_DATA_CONTROL_BYTE      0x40
#undef START_CHARACTER
#define START_CHARACTER             0x00
#define FONT_CHARACTER_COUNT        (sizeof(font6x8) / CHARACTER_WIDTH)

/**
 * @brief Sends a single command byte to the SSD1306 controller.
 */
static void OLED_SendCommand(uint8_t command) {
    I2C1_WriteRegisterByte(OLED_SSD1306_ADDRESS, OLED_COMMAND_CONTROL_BYTE, command);
}

/**
 * @brief Sets the page address for upcoming data writes.
 */
static void OLED_SetPageAddress(uint8_t page) {
    OLED_SendCommand(0xB0 | (page & 0x07));
}

/**
 * @brief Resets the column pointer to a specific column.
 */
static void OLED_SetColumnAddress(uint8_t col) {
    OLED_SendCommand(0x00 | (col & 0x0F));
    OLED_SendCommand(0x10 | ((col & 0xF0) >> 4));
}

/**
 * @brief Display initialization sequence.
 * Follows the manufacturer's recommended power-on sequence.
 */
void OLED_Init(void) {
    const uint8_t init_cmds[] = {
        0xAE, /* Display OFF */
        0xA8, 0x3F, /* Set Multiplex Ratio (64MUX) */
        0xD3, 0x00, /* Set Display Offset */
        0x40, /* Set Display Start Line (Address 0) */
        0x20, 0x02, /* Memory Addressing Mode: Page Addressing */
        0xA1, /* Segment Re-map: Column 127 is mapped to SEG0 */
        0xC8, /* COM Output Scan Direction: Remapped mode */
        0xDA, 0x12, /* COM Pins Hardware Config: Alternative */
        0x81, 0x7F, /* Set Contrast (0x00-0xFF) */
        0xA4, /* Entire Display ON: Resume to RAM content */
        0xA6, /* Set Normal Display (Not inverse) */
        0xD5, 0x80, /* Set Display Clock Divide Ratio/Oscillator Freq */
        0x8D, 0x14, /* Enable Charge Pump (Required for 3.3V operation) */
        0xAF  /* Display ON */
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

/**
 * @brief Flushes a byte array directly to an OLED page.
 */
void OLED_ArrayToPage(const uint8_t* data, uint8_t length, uint8_t page) {
    if (length > OLED_WIDTH) length = OLED_WIDTH;
    
    uint8_t i2cData[OLED_WIDTH + 1];
    i2cData[0] = OLED_DATA_CONTROL_BYTE;
    memcpy(&i2cData[1], data, length);

    OLED_SetPageAddress(page);
    OLED_SetColumnAddress(0);
    I2C1_WriteArray(OLED_SSD1306_ADDRESS, i2cData, length + 1);
}

/**
 * @brief Renders a string using the font table.
 * 
 * @param text The string to display.
 * @param page The vertical page (0-7).
 * @param fillWithBlanks If true, clears the rest of the line.
 * @param invert If true, renders white text on black background.
 */
void OLED_StringToPage(const char* text, uint8_t page, bool fillWithBlanks, bool invert) {
    uint8_t i2cData[OLED_WIDTH + 1];
    i2cData[0] = OLED_DATA_CONTROL_BYTE;
    uint8_t col = 0;
    
    /* 5x7 font + 1px spacing = 6px stride */
    uint8_t stride = CHARACTER_WIDTH + 1;

    memset(&i2cData[1], invert ? 0xFF : 0x00, OLED_WIDTH);

    for (uint16_t i = 0; text[i] != '\0' && (col + stride) <= OLED_WIDTH; i++) {
        if (text[i] == '\r' || text[i] == '\n') break;
        
        uint32_t charIdx = (uint32_t)(uint8_t)text[i] - START_CHARACTER;
        if (charIdx >= FONT_CHARACTER_COUNT) charIdx = (uint32_t)'?' - START_CHARACTER;
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
