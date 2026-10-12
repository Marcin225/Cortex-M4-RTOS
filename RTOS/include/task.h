#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "list.h"
#include "stddef.h"

typedef void (*OSThreadHandler)(void);

// Thread Control Block
typedef struct {
    uint32_t *sp; // stack pointer

    uint8_t priority;
    OS_ListItem stateNode;

}OSThread;

void OS_Thread_Create(OSThread *threadControlBlock, OSThreadHandler threadHandler, void *stkMem, size_t stkSize);

#endif