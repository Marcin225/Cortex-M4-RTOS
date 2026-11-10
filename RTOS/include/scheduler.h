#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"
#include "stddef.h"
#include <stdint.h>

#define OS_MAX_PRIORITIES                       16

extern OSThread * volatile OS_Current;
extern OSThread * volatile OS_Next;

void OS_Init(void);
void OS_Start(void);

void OS_Scheduler_AddThreadToReadyList(OSThread *threadControlBlock, uint8_t priority);

void OS_Scheduler_RemoveThreadFromReadyList(OSThread *threadControlBlock);
void OS_Scheduler_InsertThreadToReadyList(OSThread *threadControlBlock);

void OS_IncrementTick(void);
uint32_t OS_GetTickCount(void);

void OS_Delay(uint32_t tickCount);

void OS_Scheduler_RequestContextSwitch(void);
void OS_ChooseNextThread(void);

#endif