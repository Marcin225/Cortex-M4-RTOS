#include "mutex.h"
#include "list.h"
#include "port.h"
#include "rtos_assert.h"
#include "scheduler.h"
#include "task.h"
#include "types.h"
#include <stdint.h>
#include <stdbool.h>

// initializes an unlocked mutex with an empty wait list
void OS_Mutex_Init(OS_Mutex_t *mutex) {
    OS_ASSERT(mutex != NULL);

    mutex->owner = NULL;
    OS_List_Init(&mutex->waitList);
}

// must be called while the kernel critical section is active
// updates the effective thread priority according to its current scheduler state
static void OS_ChangeThreadPriority(OSThread *threadControlBlock, uint8_t newPriority) {
    OS_ASSERT(threadControlBlock != NULL);
    OS_ASSERT(newPriority < OS_MAX_PRIORITIES);

    switch (threadControlBlock->state) {
        case OS_THREAD_READY: {
            // move the thread to the ready list matching its new priority
            OS_Scheduler_RemoveThreadFromReadyList(threadControlBlock);
            threadControlBlock->priority = newPriority;
            OS_Scheduler_InsertThreadToReadyList(threadControlBlock);

            break;
        }
        case OS_THREAD_DELAYED: {
            // delayed lists are ordered by wake tick, so only the priority changes
            threadControlBlock->priority = newPriority;

            break;
        }
        case OS_THREAD_BLOCKED: {
            // reinsert the blocked thread so the wait list remains priority ordered
            OS_List *waitList = threadControlBlock->stateNode.container;
            OS_ASSERT(waitList != NULL);

            OS_List_Remove(waitList, &threadControlBlock->stateNode);
            threadControlBlock->priority = newPriority;
            OS_List_InsertByPriority(waitList, &threadControlBlock->stateNode);

            break;
        } 
        default: {
            OS_ASSERT(false);
            break;
        }
    }
}

void OS_Mutex_Take(OS_Mutex_t *mutex) {
    OS_ASSERT(mutex != NULL);
    OS_ASSERT(OS_Current != NULL);

    uint32_t criticalStatus = OS_Port_EnterCritical();

    // non-recursive mutex cannot be taken again by its current owner
    OS_ASSERT(mutex->owner != OS_Current);

    // acquire the mutex immediately if it is currently unlocked
    if (mutex->owner == NULL) {
        mutex->owner = OS_Current;
        OS_Current->mutexesHeldCtr++;

        OS_Port_ExitCritical(criticalStatus);

        return;
    }

    // apply priority inheritance if the waiting thread has a higher priority
    if (OS_Current->priority > mutex->owner->priority) {
        OS_ChangeThreadPriority(mutex->owner, OS_Current->priority);
    }

    // block the current thread until ownership is handed over by the mutex owner
    OS_Scheduler_RemoveThreadFromReadyList(OS_Current);
    OS_List_InsertByPriority(&mutex->waitList, &OS_Current->stateNode);

    OS_Current->state = OS_THREAD_BLOCKED;

    OS_Scheduler_RequestContextSwitch();

    OS_Port_ExitCritical(criticalStatus);
}

OS_Status_t OS_Mutex_Give(OS_Mutex_t *mutex) {
    OS_ASSERT(mutex != NULL);
    OS_ASSERT(OS_Current != NULL);

    uint32_t criticalStatus = OS_Port_EnterCritical();

    // only the current mutex owner is allowed to release it
    if (mutex->owner != OS_Current) {
        OS_Port_ExitCritical(criticalStatus);

        return OS_ERROR_NOT_OWNER;
    }

    bool contextSwitchRequired = false;

    OSThread *oldMutexOwner = mutex->owner;
    oldMutexOwner->mutexesHeldCtr--;

    // unlock the mutex if no thread is waiting for it
    if (mutex->waitList.numberOfItems == 0) {
        mutex->owner = NULL;

    } else {
        // directly transfer ownership to the highest-priority waiting thread
        OSThread *waitingTask = mutex->waitList.endNode.next->owner;
        
        OS_List_Remove(&mutex->waitList, mutex->waitList.endNode.next);
        mutex->owner = waitingTask;
        waitingTask->mutexesHeldCtr++;

        OS_Scheduler_InsertThreadToReadyList(waitingTask);

        // request a switch if the new owner outranks the current thread
        if (OS_Current->priority < waitingTask->priority) {
            contextSwitchRequired = true;
        }
    }

    // restore the base priority after the thread releases its last mutex
    if (oldMutexOwner->mutexesHeldCtr == 0 && oldMutexOwner->priority != oldMutexOwner->basePriority) {
        OS_ChangeThreadPriority(oldMutexOwner, oldMutexOwner->basePriority);

        contextSwitchRequired = true;
    }

    if (contextSwitchRequired) {
        OS_Scheduler_RequestContextSwitch();
    }

    OS_Port_ExitCritical(criticalStatus);

    return OS_OK;
}