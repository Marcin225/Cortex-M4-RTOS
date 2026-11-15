#ifndef OS_MUTEX_H
#define OS_MUTEX_H

#include "list.h"
#include "task.h"
#include "types.h"

typedef struct {
    OSThread *owner;
    OS_List waitList;

}OS_Mutex_t;

void OS_Mutex_Init(OS_Mutex_t *mutex);

void OS_Mutex_Take(OS_Mutex_t *mutex);
OS_Status_t OS_Mutex_Give(OS_Mutex_t *mutex);

#endif