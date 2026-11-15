#include "queue.h"
#include "list.h"
#include "port.h"
#include "rtos_assert.h"
#include "scheduler.h"
#include "task.h"
#include "types.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// initializes a blocking queue with separate wait lists for readers and writers
void OS_Queue_Init(OS_Queue_t *queue, void *buffer, uint16_t maxItems, size_t itemSize) {
    OS_ASSERT(queue != NULL);
    OS_ASSERT(buffer != NULL);
    OS_ASSERT(maxItems > 0);
    OS_ASSERT(itemSize > 0);

    OS_List_Init(&queue->writeWaitList);
    OS_List_Init(&queue->readWaitList);

    queue->buffer = buffer;
    queue->head = 0;
    queue->tail = 0;

    queue->itemSize = itemSize;
    queue->maxItems = maxItems;
    queue->numberOfItems = 0;
}

void OS_Queue_PutItem(OS_Queue_t *queue, void *putItemPtr) {
    OS_ASSERT(queue != NULL);
    OS_ASSERT(putItemPtr != NULL); 

    // retry after the thread is unblocked until the item can be inserted
    for (;;) {

        uint32_t criticalStatus = OS_Port_EnterCritical();

        // block the current thread while the queue is full
        if (queue->numberOfItems == queue->maxItems) {
            OS_Scheduler_RemoveThreadFromReadyList(OS_Current);
            OS_List_InsertByPriority(&queue->writeWaitList, &OS_Current->stateNode);

            OS_Current->state = OS_THREAD_BLOCKED;

            OS_Scheduler_RequestContextSwitch();

            OS_Port_ExitCritical(criticalStatus);

        }else {
            // copy the item into the next free queue slot
            uint8_t *destAddress = (uint8_t *)queue->buffer + queue->head * queue->itemSize;

            memcpy(destAddress, putItemPtr, queue->itemSize);

            queue->numberOfItems++;
            queue->head++;
            
            if (queue->head == queue->maxItems) queue->head = 0;

            // wake the highest-priority reader waiting for data
            if (queue->readWaitList.numberOfItems != 0) {
                OSThread *waitingTask = queue->readWaitList.endNode.next->owner;

                OS_List_Remove(&queue->readWaitList, queue->readWaitList.endNode.next);
                OS_Scheduler_InsertThreadToReadyList(waitingTask);

                if (waitingTask->priority > OS_Current->priority) {
                    OS_Scheduler_RequestContextSwitch();
                }
            }
            OS_Port_ExitCritical(criticalStatus);

            return;
        } 
    }
}

void OS_Queue_GetItem(OS_Queue_t *queue, void *getItemPtr) {
    OS_ASSERT(queue != NULL);
    OS_ASSERT(getItemPtr != NULL);

    // retry after the thread is unblocked until an item can be received
    for (;;) {

        uint32_t criticalStatus = OS_Port_EnterCritical();

        // block the current thread while the queue is empty
        if (queue->numberOfItems == 0) {
            OS_Scheduler_RemoveThreadFromReadyList(OS_Current);
            OS_List_InsertByPriority(&queue->readWaitList, &OS_Current->stateNode);

            OS_Current->state = OS_THREAD_BLOCKED;

            OS_Scheduler_RequestContextSwitch();

            OS_Port_ExitCritical(criticalStatus);
            
        }else { 
            // copy the next queued item to the destination buffer
            uint8_t *sourceAddress = (uint8_t *)queue->buffer + queue->tail * queue->itemSize;
            memcpy(getItemPtr, sourceAddress, queue->itemSize);

            queue->numberOfItems--;
            queue->tail++;

            if (queue->tail == queue->maxItems) queue->tail = 0; 

            // wake the highest-priority writer waiting for free space
            if (queue->writeWaitList.numberOfItems != 0) {
                OSThread *waitingTask = queue->writeWaitList.endNode.next->owner;

                OS_List_Remove(&queue->writeWaitList, queue->writeWaitList.endNode.next);
                OS_Scheduler_InsertThreadToReadyList(waitingTask);

                if (waitingTask->priority > OS_Current->priority) {
                    OS_Scheduler_RequestContextSwitch();
                }

            }
            OS_Port_ExitCritical(criticalStatus);

            return;
        }
    }
}