#include "scheduler.h"
#include "port.h"
#include "rtos_assert.h"
#include "rtos_config.h"
#include "task.h"
#include <stdint.h>
#include "list.h"
#include "types.h"

#define IDLE_THREAD_STACK_SIZE                  40

static volatile uint32_t OS_TickCtr;

// pointers to the currently running thread and the next selected
OSThread * volatile OS_Current = NULL;
OSThread * volatile OS_Next = NULL;

static OS_List readyList[OS_MAX_PRIORITIES]; // each priority level has its own ready list
static OS_List delayedList;
static OS_List overflowList;
static OS_List *delayedListPointer;
static OS_List *overflowListPointer;


static uint32_t readyListBitMap = 0; // bitmap used to quickly find the highest-priority ready thread

_Alignas(8) static uint32_t idleThreadStack[IDLE_THREAD_STACK_SIZE];

static OSThread idleThread;

static void OS_IdleThread(void);

// initializes scheduler state, lists, port layer and the idle thread
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

// starts the scheduler and requests the first context switch
void OS_Start(void) {
    OS_ASSERT(readyListBitMap > 0);

    OS_Port_EnableTick();

    uint32_t criticalStatus = OS_Port_EnterCritical();

    OS_Scheduler_RequestContextSwitch();

    OS_Port_ExitCritical(criticalStatus);
}

// must be called from SysTick while the kernel critical section is active
void OS_IncrementTick(void) {
    // swap delayed lists when the tick counter wraps around
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
        readyListBitMap &= ~(1UL << threadControlBlock->priority); // clear the priority bit if no thread remains at this priority
    }
}

// must be called while the kernel critical section is active
void OS_Scheduler_InsertThreadToReadyList(OSThread *threadControlBlock) {
    OS_List_InsertEnd(&readyList[threadControlBlock->priority], &threadControlBlock->stateNode);

    readyListBitMap |= (1UL << threadControlBlock->priority); // set the priority bit in the ready bitmap

    threadControlBlock->state = OS_THREAD_READY;
}

// blocks current thread for the specified number of ticks
// 1 system tick equals to 1 ms
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

    // if wakeTick wrapped around -> place the thread in the overflow list
    if (wakeTick < OS_GetTickCount()) {
        OS_List_InsertByTick(overflowListPointer, &(OS_Current->stateNode));

    }else {
        OS_List_InsertByTick(delayedListPointer, &(OS_Current->stateNode));
    }

    OS_Current->state = OS_THREAD_DELAYED;
  
    OS_Scheduler_RequestContextSwitch();
    OS_Port_ExitCritical(criticalStatus);
}

// selects the next thread and requests PendSV if a context switch is required
void OS_Scheduler_RequestContextSwitch(void) {
    OS_ChooseNextThread();

    if (OS_Current != OS_Next) {
        OS_Port_RequestContextSwitch();
    }
}

void OS_ChooseNextThread(void) {

    // move all expired delayed threads back to the ready lists
    while (delayedListPointer->numberOfItems > 0) {
        OS_ListItem *firstDelayedNode = (OS_ListItem *)delayedListPointer->endNode.next;

        OSThread *firstDelayedNodeTCB = (OSThread *)firstDelayedNode->owner;

        if (firstDelayedNode->tickCount <= OS_TickCtr) {
            OS_List_Remove(delayedListPointer, firstDelayedNode);
            OS_Scheduler_InsertThreadToReadyList(firstDelayedNodeTCB);

        }else {
            break;
        }
    }

    uint8_t priorityIndex = OS_Port_CalculatePriorityIdx(readyListBitMap);

    // select the next thread from the highest-priority ready list
    readyList[priorityIndex].currentNode = readyList[priorityIndex].currentNode->next;

    // move past the list end marker
    if (readyList[priorityIndex].currentNode == &(readyList[priorityIndex].endNode)) {
        readyList[priorityIndex].currentNode = readyList[priorityIndex].currentNode->next;
    }

    if (OS_Current != NULL) {
        OS_Task_CheckStackPattern(OS_Current);
    }

    OS_Next = readyList[priorityIndex].currentNode->owner;
}

// idle thread runs whenever no other thread is ready
static void OS_IdleThread(void) {
    for (;;) {
        OS_Port_WaitForInterrupt();
    }
}

// helper functions used only by host-side unit tests
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