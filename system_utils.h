#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H

#include <stdint.h>

void System_Init(void);
void System_DelayMs(uint32_t ms);
uint32_t System_GetTicks(void);

#endif // SYSTEM_UTILS_H
