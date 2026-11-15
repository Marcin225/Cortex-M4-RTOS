#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "list.h"
#include <stddef.h>
#include "types.h"
#include <stdnoreturn.h>

typedef void (*OSThreadHandler)(void);

// Thread Control Block
typedef struct {
    uint32_t *sp; // stack pointer

    uint32_t *stackBase; // lowest address of the thread stack
    size_t stackSize;

    uint8_t priority; // current effective priority
    uint8_t basePriority; // original priority used for priority inheritance
    
    uint8_t mutexesHeldCtr;
    OS_ThreadState_t state;

    OS_ListItem stateNode; // list node used by ready, delayed and synchronization wait lists

}OSThread;

void OS_Thread_Create(OSThread *threadControlBlock, OSThreadHandler threadHandler, uint8_t priority, void *stkMem, size_t stkSize);
void OS_Task_CheckStackPattern(OSThread *threadControlBlock);
size_t OS_GetHighWaterMark(void);


noreturn void OS_StackOverflowHandler(void);

#endif