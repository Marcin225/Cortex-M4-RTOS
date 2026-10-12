#ifndef SHEDULER_H
#define SHEDULER_H

#include "task.h"
#include "stddef.h"
#include <stdint.h>

#define MAX_THREADS                 32
#define IDLE_THREAD_STACK_SIZE      40

extern uint32_t idleThreadStack[IDLE_THREAD_STACK_SIZE];
extern OSThread idleThread;

extern OSThread * volatile OS_Current;
extern OSThread * volatile OS_Next;

// extern OSThread *threadPointerList[MAX_THREADS];
extern uint8_t threadNumber;
// extern uint8_t threadIdx;

void OS_IncrementTick();
uint32_t OS_GetTickCount();

void OS_Delay(uint32_t tickCount);
void idle_Thread();
void OS_ChooseNextThread();

#endif