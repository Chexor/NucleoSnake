#ifndef BUTTONS_H
#define BUTTONS_H

#include "stm32f091xc.h"
#include "stdbool.h"

void Buttons_Init(void);
bool SW1_IsActive(void);
bool SW2_IsActive(void);
bool SW3_IsActive(void);
bool SW4_IsActive(void);
bool UserButton_IsActive(void);

#endif // BUTTONS_H
