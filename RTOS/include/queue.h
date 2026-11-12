#ifndef QUEUE_H
#define QUEUE_H

#include "list.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    OS_List writeWaitList;
    OS_List readWaitList;

    void *buffer;
    uint32_t head;
    uint32_t tail;

    size_t itemSize;
    uint16_t maxItems;
    uint16_t numberOfItems;

}OS_Queue_t;

void OS_Queue_Init(OS_Queue_t *queue, void *buffer, uint16_t maxItems, size_t itemSize);

void OS_Queue_PutItem(OS_Queue_t *queue, void *putItemPtr);
void OS_Queue_GetItem(OS_Queue_t *queue, void *getItemPtr);

#endif