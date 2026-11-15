#include "list.h"
#include "stddef.h"
#include "rtos_assert.h"
#include "task.h"
#include <stdint.h>

void OS_List_Init(OS_List *list) {
    OS_ASSERT(list != NULL);

    list->numberOfItems = 0;

    list->currentNode = &(list->endNode);

    list->endNode.next = &(list->endNode);
    list->endNode.prev = &(list->endNode);
    list->endNode.tickCount = UINT32_MAX;
}

void OS_List_InsertByTick(OS_List *list, OS_ListItem *element) {
    OS_ASSERT(list != NULL);
    OS_ASSERT(element != NULL);
    OS_ASSERT(element != &(list->endNode));
    
    OS_ListItem *listIterator = list->endNode.next;

    // keep the list ordered by increasing tick count
    while (listIterator != &(list->endNode)) {
        if (listIterator->tickCount > element->tickCount) {
            break;
        }

        listIterator = listIterator->next;
    }

    element->next = listIterator;
    element->prev = listIterator->prev;

    listIterator->prev->next = element;
    listIterator->prev = element;
    element->container = list;

    list->numberOfItems++;
}

void OS_List_InsertByPriority(OS_List *list, OS_ListItem *element) {
    OS_ASSERT(list != NULL);
    OS_ASSERT(element != NULL);
    OS_ASSERT(element != &(list->endNode));

    OS_ListItem *listIterator = list->endNode.next;
    OSThread *elementTask = element->owner;

    // keep the highest-priority thread at the beginning of the list
    while (listIterator != &(list->endNode)) {
        OSThread *currentListTask = listIterator->owner;

        if (currentListTask->priority < elementTask->priority) {
            break;
        }

        listIterator = listIterator->next;
    }

    element->next = listIterator;
    element->prev = listIterator->prev;

    listIterator->prev->next = element;
    listIterator->prev = element;
    element->container = list;

    list->numberOfItems++;
}

void OS_List_InsertEnd(OS_List *list, OS_ListItem *element) {
    OS_ASSERT(list != NULL);
    OS_ASSERT(element != NULL);
    OS_ASSERT(element != &(list->endNode));

    element->next = &(list->endNode);
    element->prev = list->endNode.prev;

    element->prev->next = element;
    list->endNode.prev = element;
    element->container = list;

    list->numberOfItems++;
}

void OS_List_Remove(OS_List *list, OS_ListItem *element) {
    OS_ASSERT(list != NULL);
    OS_ASSERT(element != NULL);
    OS_ASSERT(list->numberOfItems > 0);
    OS_ASSERT(element != &(list->endNode));

    if (list->currentNode == element) {
        list->currentNode = list->currentNode->prev;
    }

    element->next->prev = element->prev;
    element->prev->next = element->next;

    element->next = NULL;
    element->prev = NULL;
    element->container = NULL;

    list->numberOfItems--;
}