#include "queue.h"
#include "list.h"
#include "port.h"
#include "rtos_assert.h"
#include "scheduler.h"
#include "task.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>


void OS_Queue_Init(OS_Queue_t *queue, void *buffer, uint16_t maxItems, size_t itemSize) {
    OS_ASSERT(queue != NULL);
    OS_ASSERT(buffer != NULL);
    OS_ASSERT(maxItems <= UINT16_MAX);

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

    for (;;) {

        uint32_t criticalStatus = OS_Port_EnterCritical();

        if (queue->numberOfItems == queue->maxItems) {
            OS_Scheduler_RemoveThreadFromReadyList(OS_Current);
            OS_List_InsertEnd(&queue->writeWaitList, &OS_Current->stateNode);

            OS_Scheduler_RequestContextSwitch();

            OS_Port_ExitCritical(criticalStatus);

        }else {
            uint8_t *destAddress = (uint8_t *)queue->buffer + queue->head * queue->itemSize;

            memcpy(destAddress, putItemPtr, queue->itemSize);

            queue->numberOfItems++;
            queue->head++;
            
            if (queue->head == queue->maxItems) queue->head = 0;

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

    for (;;) {

        uint32_t criticalStatus = OS_Port_EnterCritical();

        if (queue->numberOfItems == 0) {
            OS_Scheduler_RemoveThreadFromReadyList(OS_Current);
            OS_List_InsertEnd(&queue->readWaitList, &OS_Current->stateNode);

            OS_Scheduler_RequestContextSwitch();

            OS_Port_ExitCritical(criticalStatus);
            
        }else {
            uint8_t *sourceAddress = (uint8_t *)queue->buffer + queue->tail * queue->itemSize;
            memcpy(getItemPtr, sourceAddress, queue->itemSize);

            queue->numberOfItems--;
            queue->tail++;

            if (queue->tail == queue->maxItems) queue->tail = 0; 

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