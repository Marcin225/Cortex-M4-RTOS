#include "semaphore.h"
#include "list.h"
#include "port.h"
#include "rtos_assert.h"
#include "task.h"
#include "scheduler.h"
#include "types.h"
#include <stdint.h>

// common initialization helper used by both binary and counting semaphores
static void OS_Semaphore_Init(OS_Semaphore_t *semaphore, uint8_t maxCount) {
    OS_ASSERT(semaphore != NULL);
    OS_ASSERT(maxCount < 256);

    semaphore->count = maxCount;
    semaphore->maxCount = maxCount;

    OS_List_Init(&semaphore->waitList);
}

// initializes a binary semaphore with a maximum count of 1
void OS_SemaphoreBin_Init(OS_Semaphore_t *semaphore) {
    OS_ASSERT(semaphore != NULL);

    OS_Semaphore_Init(semaphore, 1);
}

// initializes a counting semaphore with the specified maximum count
void OS_SemaphoreCount_Init(OS_Semaphore_t *semaphore, uint8_t maxCount) {
    OS_ASSERT(semaphore != NULL);
    OS_ASSERT(maxCount < 256);

    OS_Semaphore_Init(semaphore, maxCount);
}

void OS_Semaphore_Take(OS_Semaphore_t *semaphore) {
    OS_ASSERT(semaphore != NULL);

    uint32_t criticalStatus = OS_Port_EnterCritical();

    // acquire the semaphore immediately if a token is available
    if (semaphore->count > 0) {
        semaphore->count--;

    }else {
        // block the current thread until the semaphore is given
        OS_Scheduler_RemoveThreadFromReadyList(OS_Current);
        OS_List_InsertByPriority(&semaphore->waitList, &OS_Current->stateNode);
        OS_Current->state = OS_THREAD_BLOCKED;

        OS_Scheduler_RequestContextSwitch();
    }

    OS_Port_ExitCritical(criticalStatus);
}

void OS_Semaphore_Give(OS_Semaphore_t *semaphore) {
    OS_ASSERT(semaphore != NULL);

    uint32_t criticalStatus = OS_Port_EnterCritical();

    // increment the semaphore count if no thread is waiting
    if (semaphore->waitList.numberOfItems == 0) {
        if (semaphore->count < semaphore->maxCount) {
            semaphore->count++;
        }

    } else {
        // wake the highest-priority waiting thread and switch if it outranks the current thread
        OSThread *waitingThread = (OSThread *)semaphore->waitList.endNode.next->owner;

        OS_List_Remove(&semaphore->waitList, semaphore->waitList.endNode.next);
        OS_Scheduler_InsertThreadToReadyList(waitingThread);

        if (OS_Current->priority < waitingThread->priority) {
            OS_Scheduler_RequestContextSwitch();
        }
    } 

    OS_Port_ExitCritical(criticalStatus);
}