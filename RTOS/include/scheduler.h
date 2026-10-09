#ifndef SHEDULER_H
#define SHEDULER_H

#include "task.h"
#include "stddef.h"
#include <stdint.h>

#define MAX_THREADS         32

extern OSThread * volatile OS_Current;
extern OSThread * volatile OS_Next;

extern OSThread *threadPointerList[MAX_THREADS];
extern uint8_t threadNumber;
extern uint8_t threadIdx;

void OS_ChooseNextThread();

#endif