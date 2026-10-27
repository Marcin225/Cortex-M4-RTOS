#ifndef RTOS_CONFIG_H
#define RTOS_CONFIG_H

#define OS_NVIC_PRIO_BITS                       4
#define OS_MAX_SYSCALL_INTERRUPT_PRIORITY       6 // RTOS API interrupt threshold

#define OS_BASEPRI_MASK                         (OS_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - OS_NVIC_PRIO_BITS)) // encoded BASEPRI threshold

#endif