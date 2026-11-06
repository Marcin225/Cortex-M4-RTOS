#include "task.h"
#include "port.h"
#include "rtos_assert.h"
#include "scheduler.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "stm32l4xx.h"

_Static_assert(offsetof(OSThread, stackBase) == 4, "stackBase offset mismatch");

void OS_Thread_Create(OSThread *threadControlBlock, OSThreadHandler threadHandler, uint8_t priority, void *stkMem, size_t stkSize) {
    OS_ASSERT(threadControlBlock != NULL);
    OS_ASSERT(threadHandler != NULL);
    OS_ASSERT(priority >= 0 && priority < 16);
    OS_ASSERT(stkMem != NULL);
    OS_ASSERT(stkSize > 0);

    // paint stack to help detect stack overflow and measure high-water mark
    memset(stkMem, 0xA5, stkSize);

    threadControlBlock->sp = OS_Port_InitStack(threadHandler, stkMem, stkSize);

    threadControlBlock->stackBase = stkMem;
    threadControlBlock->stackSize = stkSize;
    threadControlBlock->priority = priority;
    threadControlBlock->stateNode.next = NULL;
    threadControlBlock->stateNode.prev = NULL;
    threadControlBlock->stateNode.owner = threadControlBlock;
    threadControlBlock->stateNode.tickCount = 0;
    
    OS_Scheduler_AddThreadToReadyList(threadControlBlock, priority);

}

size_t OS_GetHighWaterMark(void) {
    OS_ASSERT(OS_Current != NULL);

    size_t freeBytesCtr = 0;

    for (size_t i = 0; i < OS_Current->stackSize / sizeof(uint32_t); i++) {
        if (OS_Current->stackBase[i] == 0xA5A5A5A5) {
            freeBytesCtr += sizeof(uint32_t);
        }else {
            break;
        }
    }

    return freeBytesCtr;
}

void OS_Task_CheckStackPattern(OSThread *threadControlBlock) {
    OS_ASSERT(threadControlBlock != NULL);
    
    OS_ASSERT(threadControlBlock->stackBase[0] == 0xA5A5A5A5);
    OS_ASSERT(threadControlBlock->stackBase[1] == 0xA5A5A5A5);
    OS_ASSERT(threadControlBlock->stackBase[2] == 0xA5A5A5A5);
    OS_ASSERT(threadControlBlock->stackBase[3] == 0xA5A5A5A5);
}

__attribute__((noreturn))
void OS_StackOverflowHandler(void) {
    __disable_irq(); 
    __BKPT(0); // stop the debugger at this line

    while (1) { 

    }
}