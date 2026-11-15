#ifndef LIST_H
#define LIST_H

#include <stdint.h>


// intrusive doubly linked list used by the RTOS scheduler

// each thread owns its OS_ListItem inside the TCB, so list operations
// do not allocate memory dynamically

// the list uses a sentinel endNode to simplify insert and remove operations
// currentNode is used to keep the current position when iterating through
// ready lists for scheduling

// owner points back to the object containing the list item (OSThread)
// container points to the list in which the item is currently stored
 
typedef struct OS_List_t OS_List;

typedef struct OS_ListItem_t {
    struct OS_ListItem_t *next;
    struct OS_ListItem_t *prev;
    void *owner;
    OS_List *container;

    uint32_t tickCount;
    
}OS_ListItem;

struct OS_List_t {
    uint32_t numberOfItems;
    OS_ListItem *currentNode;
    OS_ListItem endNode;
};

void OS_List_Init(OS_List *list);
void OS_List_InsertByTick(OS_List *list, OS_ListItem *element);
void OS_List_InsertByPriority(OS_List *list, OS_ListItem *element);
void OS_List_InsertEnd(OS_List *list, OS_ListItem *element);
void OS_List_Remove(OS_List *list, OS_ListItem *element);

#endif