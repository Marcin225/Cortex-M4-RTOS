#ifndef PORT_H
#define PORT_H

#include <stdint.h>
#include "stddef.h"
#include "task.h"

typedef void (*OSThreadHandler)(void);

void OSThread_Create(OSThread *ThreadControlBlock, OSThreadHandler threadHandler, void *stkMem, size_t stkSize);
void OS_Init(void);

#endif