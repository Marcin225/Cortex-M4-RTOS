#include "port.h"
#include <stdint.h>

uint32_t *OS_Port_InitStack(OSThreadHandler threadHandler, void *stkMem, size_t stkSize) {
    (void)threadHandler;

    return (uint32_t *)((uint8_t *)stkMem + stkSize);
}

void OS_Port_DisableInterrupts(void) {
}

void OS_Port_Breakpoint(void) {
}

void OS_Port_Init(void) {
}

void OS_Port_EnableTick(void) {
}

void OS_Port_RequestContextSwitch(void) {
}

uint32_t OS_Port_EnterCritical(void) {
    return 0;
}

void OS_Port_ExitCritical(uint32_t prevBASEPRI) {
    (void)prevBASEPRI;
}

uint8_t OS_Port_CalculatePriorityIdx(uint32_t readyListBitMap) {
    uint8_t priority = 0;

    while (readyListBitMap >>= 1U) {
        priority++;
    }

    return priority;
}

void OS_Port_WaitForInterrupt(void) {
}