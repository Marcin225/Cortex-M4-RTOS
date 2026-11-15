#ifndef RTOS_CONFIG_H
#define RTOS_CONFIG_H

// number of implemented NVIC priority bits on Cortex-M4
#define OS_NVIC_PRIO_BITS                       4

// highest interrupt priority allowed to call RTOS API functions
#define OS_MAX_SYSCALL_INTERRUPT_PRIORITY       6 

// BASEPRI value used to mask RTOS-aware interrupts
#define OS_BASEPRI_MASK                         (OS_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - OS_NVIC_PRIO_BITS)) // encoded BASEPRI threshold

// byte offset of stackBase inside OSThread, used by PendSV assembly
#define OS_STACK_BASE_OFFSET                    4

// number of supported thread priority levels (1-15)
// priority 0 is reserved for the idle thread
#define OS_MAX_PRIORITIES                       16

#endif