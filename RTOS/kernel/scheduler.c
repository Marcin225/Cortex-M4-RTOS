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
static uint32_t readyListBitMap = 0;

__attribute__((aligned(8)))
static uint32_t idleThreadStack[IDLE_THREAD_STACK_SIZE];

static OSThread idleThread;

static void OS_IdleThread(void);

void OS_Init(void) {
    OS_TickCtr = 0;

    OS_Current = NULL;
    OS_Next = NULL;

    for (int i = 0; i < OS_MAX_PRIORITIES; i++) {
        OS_List_Init(&readyList[i]);
    }
    OS_List_Init(&delayedList);

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

void OS_IncrementTick(void) {
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

    OS_Current->stateNode.tickCount = wakeTick; // ******************* overflow optimization needed ******************

    OS_List_Remove(&readyList[OS_Current->priority], &(OS_Current->stateNode));
    if (readyList[OS_Current->priority].numberOfItems == 0) {
        readyListBitMap &= ~(1 << OS_Current->priority);
    }
    OS_List_Insert(&delayedList, &(OS_Current->stateNode));
  
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

    while (delayedList.numberOfItems > 0) {
        OS_ListItem *firstDelayedNode = delayedList.endNode.next;

        OSThread *firstDelayedNodeTCB = firstDelayedNode->owner;

        if (firstDelayedNode->tickCount <= OS_TickCtr) {
            OS_List_Remove(&delayedList, firstDelayedNode);
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