#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "list.h"

// Thread Control Block
typedef struct {
    uint32_t *sp; // stack pointer

    uint8_t priority;
    OS_ListItem stateNode;

}OSThread;

#endif