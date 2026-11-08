#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "list.h"
#include "stddef.h"
#include <stdnoreturn.h>

typedef void (*OSThreadHandler)(void);

// Thread Control Block
typedef struct {
    uint32_t *sp; // stack pointer

    uint32_t *stackBase;
    size_t stackSize;

    uint8_t priority;
    OS_ListItem stateNode;

}OSThread;

void OS_Thread_Create(OSThread *threadControlBlock, OSThreadHandler threadHandler, uint8_t priority, void *stkMem, size_t stkSize);
void OS_Task_CheckStackPattern(OSThread *threadControlBlock);
size_t OS_GetHighWaterMark(void);


noreturn void OS_StackOverflowHandler(void);

#endif