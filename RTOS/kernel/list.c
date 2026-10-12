#include "list.h"
#include "port.h"
#include "rtos_assert.h"
#include <stdint.h>

OS_List blockedList;
OS_List readyList;

void OS_List_Init(OS_List *list) {
    OS_ASSERT(list != NULL);

    list->numberOfItems = 0;

    list->currentNode = &(list->endNode);

    list->endNode.next = &(list->endNode);
    list->endNode.prev = &(list->endNode);
    list->endNode.tickCount = UINT32_MAX;
}

void OS_List_Insert(OS_List *list, OS_ListItem *element) {
    OS_ASSERT(list != NULL && element != NULL && element != &(list->endNode));
    
    OS_ListItem *listIterator;

    for (listIterator = list->endNode.next; listIterator->tickCount <= element->tickCount; listIterator = listIterator->next) {
        // find node that has greater tickCount than element
    }

    element->next = listIterator;
    element->prev = listIterator->prev;

    listIterator->prev->next = element;
    listIterator->prev = element;

    list->numberOfItems++;
}

void OS_List_InsertEnd(OS_List *list, OS_ListItem *element) {
    OS_ASSERT(list != NULL && element != NULL && element != &(list->endNode));

    element->next = &(list->endNode);
    element->prev = list->endNode.prev;

    element->prev->next = element;
    list->endNode.prev = element;

    list->numberOfItems++;
}

void OS_List_Remove(OS_List *list, OS_ListItem *element) {
    OS_ASSERT(list != NULL && element != NULL && list->numberOfItems > 0 && element != &(list->endNode));

    if (list->currentNode == element) {
        list->currentNode = list->currentNode->prev;
    }

    element->next->prev = element->prev;
    element->prev->next = element->next;

    element->next = NULL;
    element->prev = NULL;

    list->numberOfItems--;
}