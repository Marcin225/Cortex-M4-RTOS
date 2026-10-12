#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"
#include "stddef.h"
#include <stdint.h>

extern OSThread * volatile OS_Current;
extern OSThread * volatile OS_Next;

extern uint8_t threadNumber;

void OS_Init(void);
void OS_Start(void);

void OS_Scheduler_AddThreadToReadyList(OSThread *threadControlBlock);

void OS_IncrementTick(void);
uint32_t OS_GetTickCount(void);

void OS_Delay(uint32_t tickCount);

void OS_RequestContextSwitch(void);
void OS_ChooseNextThread(void);

#endif