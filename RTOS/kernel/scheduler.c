#include "scheduler.h"
#include "port.h"
#include "rtos_assert.h"
#include "task.h"
#include <stdint.h>
#include "list.h"

#define IDLE_THREAD_STACK_SIZE                  40

static volatile uint32_t OS_TickCtr;

OSThread * volatile OS_Current = NULL;
OSThread * volatile OS_Next = NULL;

static OS_List readyList[OS_MAX_PRIORITIES];
static OS_List delayedList;
static OS_List overflowList;
static OS_List *delayedListPointer;
static OS_List *overflowListPointer;


static uint32_t readyListBitMap = 0;

_Alignas(8) static uint32_t idleThreadStack[IDLE_THREAD_STACK_SIZE];

static OSThread idleThread;

static void OS_IdleThread(void);

void OS_Init(void) {
    OS_TickCtr = 0;
    readyListBitMap = 0;

    OS_Current = NULL;
    OS_Next = NULL;

    for (int i = 0; i < OS_MAX_PRIORITIES; i++) {
        OS_List_Init(&readyList[i]);
    }
    OS_List_Init(&delayedList);
    OS_List_Init(&overflowList);
    delayedListPointer = &delayedList;
    overflowListPointer = &overflowList;

    OS_Port_Init();

    OS_Thread_Create(&idleThread, &OS_IdleThread, 0, idleThreadStack, sizeof(idleThreadStack));
}

void OS_Start(void) {
    OS_ASSERT(readyListBitMap > 0);

    OS_Port_EnableTick();

    uint32_t criticalStatus = OS_Port_EnterCritical();

    OS_Scheduler_RequestContextSwitch();

    OS_Port_ExitCritical(criticalStatus);
}

// must be called from SysTick while the kernel critical section is active
void OS_IncrementTick(void) {
    // check if oveflow appeared
    if (OS_TickCtr == UINT32_MAX) {
        OS_List *temp = delayedListPointer;
        delayedListPointer = overflowListPointer;
        overflowListPointer = temp;
    }
    OS_TickCtr++;
}

uint32_t OS_GetTickCount(void) {
    return OS_TickCtr;
}

// must be called while the kernel critical section is active
void OS_Scheduler_RemoveThreadFromReadyList(OSThread *threadControlBlock) {
    OS_List_Remove(&readyList[threadControlBlock->priority], &threadControlBlock->stateNode);
    if (readyList[threadControlBlock->priority].numberOfItems == 0) {
        readyListBitMap &= ~(1UL << threadControlBlock->priority);
    }
}

// must be called while the kernel critical section is active
void OS_Scheduler_InsertThreadToReadyList(OSThread *threadControlBlock) {
    OS_List_InsertEnd(&readyList[threadControlBlock->priority], &threadControlBlock->stateNode);

    readyListBitMap |= (1UL << threadControlBlock->priority);
}

void OS_Delay(uint32_t tickCount) {
    OS_ASSERT(OS_Current != NULL);
    OS_ASSERT(OS_Current != &idleThread);

    uint32_t criticalStatus = OS_Port_EnterCritical();

    uint32_t wakeTick = OS_GetTickCount() + tickCount;

    OS_Current->stateNode.tickCount = wakeTick;

    OS_List_Remove(&readyList[OS_Current->priority], &(OS_Current->stateNode));
    if (readyList[OS_Current->priority].numberOfItems == 0) {
        readyListBitMap &= ~(1UL << OS_Current->priority);
    }

    // check if oveflow appeared
    if (wakeTick < OS_GetTickCount()) {
        OS_List_Insert(overflowListPointer, &(OS_Current->stateNode));

    }else {
        OS_List_Insert(delayedListPointer, &(OS_Current->stateNode));
    }
  
    OS_Scheduler_RequestContextSwitch();
    OS_Port_ExitCritical(criticalStatus);
}

void OS_Scheduler_RequestContextSwitch(void) {
    OS_ChooseNextThread();

    if (OS_Current != OS_Next) {
        OS_Port_RequestContextSwitch();
    }
}

void OS_ChooseNextThread(void) {

    while (delayedListPointer->numberOfItems > 0) {
        OS_ListItem *firstDelayedNode = delayedListPointer->endNode.next;

        OSThread *firstDelayedNodeTCB = firstDelayedNode->owner;

        if (firstDelayedNode->tickCount <= OS_TickCtr) {
            OS_List_Remove(delayedListPointer, firstDelayedNode);
            OS_List_InsertEnd(&readyList[firstDelayedNodeTCB->priority], firstDelayedNode);
            readyListBitMap |= (1UL << firstDelayedNodeTCB->priority);

        }else {
            break;
        }
    }

    uint8_t priorityIndex = OS_Port_CalculatePriorityIdx(readyListBitMap);

    readyList[priorityIndex].currentNode = readyList[priorityIndex].currentNode->next;

    if (readyList[priorityIndex].currentNode == &(readyList[priorityIndex].endNode)) {
        readyList[priorityIndex].currentNode = readyList[priorityIndex].currentNode->next;
    }

    if (OS_Current != NULL) {
        OS_Task_CheckStackPattern(OS_Current);
    }

    OS_Next = readyList[priorityIndex].currentNode->owner;
}

static void OS_IdleThread(void) {
    for (;;) {
        OS_Port_WaitForInterrupt();
    }
}

#ifdef OS_TEST

void OS_Test_SetTickCount(uint32_t tickCount) {
    OS_TickCtr = tickCount;
}

void OS_Test_InitTickLists(void) {
    OS_List_Init(&delayedList);
    OS_List_Init(&overflowList);

    delayedListPointer = &delayedList;
    overflowListPointer = &overflowList;
}

OS_List *OS_Test_GetDelayedListPointer(void) {
    return delayedListPointer;
}

OS_List *OS_Test_GetOverflowListPointer(void) {
    return overflowListPointer;
}

#endif