#include "task.h"
#include "port.h"
#include "rtos_assert.h"
#include "scheduler.h"
#include <stdint.h>

void OS_Thread_Create(OSThread *threadControlBlock, OSThreadHandler threadHandler, uint8_t priority, void *stkMem, size_t stkSize) {
    OS_ASSERT(threadControlBlock != NULL);
    OS_ASSERT(threadHandler != NULL);
    OS_ASSERT(priority >= 0 && priority < 16);
    OS_ASSERT(stkMem != NULL);
    OS_ASSERT(stkSize > 0);

    threadControlBlock->sp = OS_Port_InitStack(threadHandler, stkMem, stkSize);

    threadControlBlock->priority = priority;
    threadControlBlock->stateNode.next = NULL;
    threadControlBlock->stateNode.prev = NULL;
    threadControlBlock->stateNode.owner = threadControlBlock;
    threadControlBlock->stateNode.tickCount = 0;
    
    OS_Scheduler_AddThreadToReadyList(threadControlBlock, priority);

}