#ifndef OLED_H
#define OLED_H

#include <stdint.h>
#include <stdbool.h>

#define OLED_SSD1306_ADDRESS        0x3C
#define OLED_WIDTH                  128
#define OLED_HEIGHT                 64
#define OLED_NUMBER_OF_PAGES        8
#define CHARACTER_WIDTH             5

void OLED_Init(void);
void OLED_FillScreen(uint8_t data);
void OLED_FillPage(uint8_t data, uint8_t page);
void OLED_StringToPage(const char* text, uint8_t page, bool fillWithBlanks, bool invert);
void OLED_ArrayToPage(const uint8_t* data, uint8_t length, uint8_t page);

#endif // OLED_H
