#ifndef OS_TYPES_H
#define OS_TYPES_H

// status codes returned by RTOS API functions
typedef enum {
    OS_OK = 0,
    OS_ERROR_NOT_OWNER = 1

}OS_Status_t;

// possible thread scheduler states
typedef enum {
    OS_THREAD_READY,
    OS_THREAD_BLOCKED,
    OS_THREAD_DELAYED

}OS_ThreadState_t;

#endif