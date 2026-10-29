#include "scheduler.h"
#include "cmsis_gcc.h"
#include "port.h"
#include "rtos_assert.h"
#include "task.h"
#include <stdint.h>
#include "list.h"

#define IDLE_THREAD_STACK_SIZE                  40
#define OS_MAX_PRIORITIES                       16

static volatile uint32_t OS_TickCtr;

OSThread * volatile OS_Current = NULL;
OSThread * volatile OS_Next = NULL;

static OS_List readyList[OS_MAX_PRIORITIES];
static OS_List delayedList;
static OS_List overflowList;
static OS_List *delayedListPointer;
static OS_List *overflowListPointer;


static uint32_t readyListBitMap = 0;

__attribute__((aligned(8)))
static uint32_t idleThreadStack[IDLE_THREAD_STACK_SIZE];

static OSThread idleThread;

static void OS_IdleThread(void);

void OS_Init(void) {
    OS_TickCtr = UINT32_MAX - 5;

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

    OS_RequestContextSwitch();

    OS_Port_ExitCritical(criticalStatus);
}

void OS_Scheduler_AddThreadToReadyList(OSThread *threadControlBlock, uint8_t priority) {
    OS_ASSERT(threadControlBlock != NULL);
    OS_ASSERT(priority >= 0 && priority < 16);

    uint32_t criticalStatus = OS_Port_EnterCritical();

    OS_List_InsertEnd(&readyList[priority], &(threadControlBlock->stateNode));
    readyListBitMap |= 1 << priority;

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

void OS_Delay(uint32_t tickCount) {
    OS_ASSERT(OS_Current != NULL);
    OS_ASSERT(OS_Current != &idleThread);

    uint32_t criticalStatus = OS_Port_EnterCritical();

    uint32_t wakeTick = OS_GetTickCount() + tickCount;

    OS_Current->stateNode.tickCount = wakeTick;

    OS_List_Remove(&readyList[OS_Current->priority], &(OS_Current->stateNode));
    if (readyList[OS_Current->priority].numberOfItems == 0) {
        readyListBitMap &= ~(1 << OS_Current->priority);
    }

    // check if oveflow appeared
    if (wakeTick < OS_GetTickCount()) {
        OS_List_Insert(overflowListPointer, &(OS_Current->stateNode));

    }else {
        OS_List_Insert(delayedListPointer, &(OS_Current->stateNode));
    }
  
    OS_RequestContextSwitch();
    OS_Port_ExitCritical(criticalStatus);
}

void OS_RequestContextSwitch(void) {
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
            readyListBitMap |= (1 << firstDelayedNodeTCB->priority);

        }else {
            break;
        }
    }

    uint8_t priorityIndex = 31 - __CLZ(readyListBitMap);

    readyList[priorityIndex].currentNode = readyList[priorityIndex].currentNode->next;

    if (readyList[priorityIndex].currentNode == &(readyList[priorityIndex].endNode)) {
        readyList[priorityIndex].currentNode = readyList[priorityIndex].currentNode->next;
    }

    OS_Next = readyList[priorityIndex].currentNode->owner;
}

static void OS_IdleThread(void) {
    for (;;) {
        __WFI();
    }
}