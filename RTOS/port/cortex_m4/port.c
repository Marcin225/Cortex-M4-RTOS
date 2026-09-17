#include "port.h"
#include <stdint.h>

void OS_Init(void) {
    // 0xE000ED20 -> System handler priority register 3 (SHPR3)
    *(volatile uint32_t *)0xE000ED20 |= 0xFFU << 16; // set PendSV priority to the lowest level
}

void OSThread_Create(OSThread *threadControlBlock, OSThreadHandler threadHandler, void *stkMem, size_t stkSize) {

    uint32_t *sp = (uint32_t *)(((uint32_t)stkMem + stkSize) & ~7U); // align stack pointer to the next lower 8-byte boundary

    // Cortex-M hardware automatically saves these registers on exception entry
    *(--sp) = 1U << 24; // PSR
    *(--sp) = (uint32_t)threadHandler; // PC
    *(--sp) = 0x13131313; // LR
    *(--sp) = 0x12121212; // R12
    *(--sp) = 0x03030303; // R3
    *(--sp) = 0x02020202; // R2
    *(--sp) = 0x01010101; // R1
    *(--sp) = 0x00000000; // R0

    // software-saved registers: manually push the remaining CPU context
    *(--sp) = 0x11111111; // R11
    *(--sp) = 0x10101010; // R10
    *(--sp) = 0x09090909; // R9
    *(--sp) = 0x08080808; // R8
    *(--sp) = 0x07070707; // R7
    *(--sp) = 0x06060606; // R6
    *(--sp) = 0x05050505; // R5
    *(--sp) = 0x04040404; // R4

    threadControlBlock->sp = sp;
}