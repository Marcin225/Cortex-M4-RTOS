#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>
#include "list.h"
#include "task.h"

typedef struct {
    uint8_t state;
    OS_List waitList;

}OS_SemaphoreBin_t;


void OS_SemaphoreBin_Init(OS_SemaphoreBin_t *semaphore);

void OS_SemaphoreBin_Take(OS_SemaphoreBin_t *semaphore);
void OS_SemaphoreBin_Give(OS_SemaphoreBin_t *semaphore);

#endif