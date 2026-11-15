#include "task.h"
#include "port.h"
#include "rtos_assert.h"
#include "rtos_config.h"
#include "scheduler.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// ensures stackBase is at offset 4 in the TCB, as expected by the PendSV stack overflow check
#ifndef OS_TEST
_Static_assert(offsetof(OSThread, stackBase) == OS_STACK_BASE_OFFSET, "stackBase offset mismatch");
#endif

// initializes a thread and inserts it into the ready list
void OS_Thread_Create(OSThread *threadControlBlock, OSThreadHandler threadHandler, uint8_t priority, void *stkMem, size_t stkSize) {
    OS_ASSERT(threadControlBlock != NULL);
    OS_ASSERT(threadHandler != NULL);
    OS_ASSERT(priority < OS_MAX_PRIORITIES);
    OS_ASSERT(stkMem != NULL);
    OS_ASSERT(stkSize > 0);

    // paint stack to help detect stack overflow and measure high-water mark
    memset(stkMem, 0xA5, stkSize);

    threadControlBlock->sp = OS_Port_InitStack(threadHandler, stkMem, stkSize);

    threadControlBlock->stackBase = stkMem;
    threadControlBlock->stackSize = stkSize;
    threadControlBlock->priority = priority;
    threadControlBlock->basePriority = priority;
    threadControlBlock->mutexesHeldCtr = 0;
    threadControlBlock->state = OS_THREAD_READY;
    threadControlBlock->stateNode.next = NULL;
    threadControlBlock->stateNode.prev = NULL;
    threadControlBlock->stateNode.owner = threadControlBlock;
    threadControlBlock->stateNode.container = NULL;
    threadControlBlock->stateNode.tickCount = 0;

    uint32_t criticalStatus = OS_Port_EnterCritical();

    OS_Scheduler_InsertThreadToReadyList(threadControlBlock);

    OS_Port_ExitCritical(criticalStatus);
}

// returns the number of unused stack bytes 
// Cortex-M uses a descending stack, so the untouched area is measured from the lowest stack address upward
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

// checks the lowest four stack words for corruption as an early stack overflow indicator
// this method is not fully reliable because a large stack allocation may overflow past
// the checked words before the pattern is detected
void OS_Task_CheckStackPattern(OSThread *threadControlBlock) {
    OS_ASSERT(threadControlBlock != NULL);
    
    OS_ASSERT(threadControlBlock->stackBase[0] == 0xA5A5A5A5);
    OS_ASSERT(threadControlBlock->stackBase[1] == 0xA5A5A5A5);
    OS_ASSERT(threadControlBlock->stackBase[2] == 0xA5A5A5A5);
    OS_ASSERT(threadControlBlock->stackBase[3] == 0xA5A5A5A5);
}

// stops execution after a stack overflow is detected
noreturn void OS_StackOverflowHandler(void) {
    OS_Port_DisableInterrupts();
    OS_Port_Breakpoint(); // stop the debugger at this line

    while (1) { 

    }
}