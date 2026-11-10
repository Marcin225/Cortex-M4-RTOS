#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>
#include "list.h"
#include "task.h"

typedef struct {
    uint8_t count;
    uint8_t maxCount;
    OS_List waitList;

}OS_Semaphore_t;

void OS_SemaphoreBin_Init(OS_Semaphore_t *semaphore);
void OS_SemaphoreCount_Init(OS_Semaphore_t *semaphore, uint8_t maxCount);

void OS_Semaphore_Take(OS_Semaphore_t *semaphore);
void OS_Semaphore_Give(OS_Semaphore_t *semaphore);

#endif