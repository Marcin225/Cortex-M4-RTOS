#include "scheduler.h"
#include "cmsis_gcc.h"
#include "port.h"
#include "task.h"
#include <stdint.h>
#include "list.h"

static volatile uint32_t OS_TickCtr;

OSThread * volatile OS_Current = NULL;
OSThread * volatile OS_Next = NULL;

// OSThread *threadPointerList[MAX_THREADS];

uint8_t threadNumber = 0;
// uint8_t threadIdx = 0;

uint32_t idleThreadStack[IDLE_THREAD_STACK_SIZE];
OSThread idleThread;

void OS_IncrementTick() {
    OS_TickCtr++;
}

uint32_t OS_GetTickCount() {
    return OS_TickCtr;
}

void OS_Delay(uint32_t tickCount) {
    uint32_t delayTicks = OS_GetTickCount() + tickCount;

    OS_Current->stateNode.tickCount = delayTicks; // ******************* overflow optimization needed ******************

    OS_List_Remove(&readyList, &(OS_Current->stateNode));
    OS_List_Insert(&blockedList, &(OS_Current->stateNode));

    __disable_irq();
    OS_RequestContextSwitch();
    __enable_irq();
}

void OS_ChooseNextThread() {

    while (blockedList.numberOfItems > 0) {
        OS_ListItem *firstBlockedNode = blockedList.endNode.next;

        if (firstBlockedNode->tickCount <= OS_TickCtr) {
            OS_List_Remove(&blockedList, firstBlockedNode);
            OS_List_InsertEnd(&readyList, firstBlockedNode);

        }else {
            break;
        }
    }

    readyList.currentNode = readyList.currentNode->next;

    if (readyList.currentNode == &(readyList.endNode)) {
        readyList.currentNode = readyList.currentNode->next;
    }

    OS_Next = readyList.currentNode->owner;

    // if (threadIdx >= threadNumber) {
    //     threadIdx = 0;
    // }
    // OS_Next = threadPointerList[threadIdx++];
}

void idle_Thread() {
    for (;;) {
        __WFI();
    }
}