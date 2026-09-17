#ifndef TASK_H
#define TASK_H

#include <stdint.h>

// Thread Control Block
typedef struct {
    uint32_t *sp; // stack pointer

}OSThread;

#endif