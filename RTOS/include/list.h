#ifndef LIST_H
#define LIST_H

#include <stdint.h>

typedef struct OS_ListItem_t {
    struct OS_ListItem_t *next;
    struct OS_ListItem_t *prev;
    void *owner;

    uint32_t tickCount;
    
}OS_ListItem;

typedef struct {
    uint32_t numberOfItems;
    OS_ListItem *currentNode;
    OS_ListItem endNode;

} OS_List;

extern OS_List blockedList;
extern OS_List readyList;

void OS_List_Init(OS_List *list);
void OS_List_Insert(OS_List *list, OS_ListItem *element);
void OS_List_InsertEnd(OS_List *list, OS_ListItem *element);
void OS_List_Remove(OS_List *list, OS_ListItem *element);

#endif