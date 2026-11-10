#include "semaphore.h"
#include "list.h"
#include "port.h"
#include "rtos_assert.h"
#include "task.h"
#include "scheduler.h"
#include <stdint.h>

void OS_SemaphoreBin_Init(OS_SemaphoreBin_t *semaphore) {
    OS_ASSERT(semaphore != NULL);

    semaphore->state = 1;
    OS_List_Init(&semaphore->waitList);
}

void OS_SemaphoreBin_Take(OS_SemaphoreBin_t *semaphore) {
    OS_ASSERT(semaphore != NULL);

    uint32_t criticalStatus = OS_Port_EnterCritical();

    if (semaphore->state == 1) {
        semaphore->state = 0;

    }else {
        OS_Scheduler_RemoveThreadFromReadyList(OS_Current);
        OS_List_InsertEnd(&semaphore->waitList, &OS_Current->stateNode);

        OS_Scheduler_RequestContextSwitch();
    }

    OS_Port_ExitCritical(criticalStatus);
}

void OS_SemaphoreBin_Give(OS_SemaphoreBin_t *semaphore) {
    OS_ASSERT(semaphore != NULL);

    uint32_t criticalStatus = OS_Port_EnterCritical();

    if (semaphore->waitList.numberOfItems == 0) {
        semaphore->state = 1;

    } else {
        OSThread *waitingThread = semaphore->waitList.endNode.next->owner;

        OS_List_Remove(&semaphore->waitList, semaphore->waitList.endNode.next);
        OS_Scheduler_InsertThreadToReadyList(waitingThread);

        if (OS_Current->priority < waitingThread->priority) {
            OS_Scheduler_RequestContextSwitch();
        }
    } 

    OS_Port_ExitCritical(criticalStatus);
}