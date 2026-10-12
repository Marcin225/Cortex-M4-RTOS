#include "scheduler.h"
#include "cmsis_gcc.h"
#include "port.h"
#include "rtos_assert.h"
#include "task.h"
#include <stdint.h>
#include "list.h"

#define IDLE_THREAD_STACK_SIZE      40

static volatile uint32_t OS_TickCtr;

OSThread * volatile OS_Current = NULL;
OSThread * volatile OS_Next = NULL;

static OS_List readyList;
static OS_List delayedList;

__attribute__((aligned(8)))
static uint32_t idleThreadStack[IDLE_THREAD_STACK_SIZE];

static OSThread idleThread;

uint8_t threadNumber = 0;

static void OS_IdleThread(void);

void OS_Init(void) {
    OS_TickCtr = 0;

    OS_Current = NULL;
    OS_Next = NULL;

    OS_List_Init(&readyList);
    OS_List_Init(&delayedList);

    OS_Port_Init();

    OS_Thread_Create(&idleThread, &OS_IdleThread, idleThreadStack, sizeof(idleThreadStack));
}

void OS_Start(void) {
    OS_ASSERT(readyList.numberOfItems > 0);

    OS_Port_EnableTick();

    __disable_irq();
    OS_RequestContextSwitch();
    __enable_irq();
}

void OS_Scheduler_AddThreadToReadyList(OSThread *threadControlBlock) {
    OS_ASSERT(threadControlBlock != NULL);

    __disable_irq();
    OS_List_InsertEnd(&readyList, &(threadControlBlock->stateNode));
    __enable_irq();
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

    __disable_irq();
    uint32_t wakeTick = OS_GetTickCount() + tickCount;

    OS_Current->stateNode.tickCount = wakeTick; // ******************* overflow optimization needed ******************

    OS_List_Remove(&readyList, &(OS_Current->stateNode));
    OS_List_Insert(&delayedList, &(OS_Current->stateNode));

    
    OS_RequestContextSwitch();
    __enable_irq();
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

        if (firstDelayedNode->tickCount <= OS_TickCtr) {
            OS_List_Remove(&delayedList, firstDelayedNode);
            OS_List_InsertEnd(&readyList, firstDelayedNode);

        }else {
            break;
        }
    }

    readyList.currentNode = readyList.currentNode->next;

    if (readyList.currentNode == &(readyList.endNode)) {
        readyList.currentNode = readyList.currentNode->next;
    }

    OS_Next = readyList.currentNode->owner;
}

static void OS_IdleThread(void) {
    for (;;) {
        __WFI();
    }
}