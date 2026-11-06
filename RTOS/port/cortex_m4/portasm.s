#include "rtos_config.h"

.syntax unified
.cpu cortex-m4
.thumb

.extern OS_Current
.extern OS_Next

.global PendSV_Handler
.type PendSV_Handler, %function
.thumb_func


PendSV_Handler:
    // enter critical section
    mov         r0, #OS_BASEPRI_MASK // move OS_BASEPRI_MASK into r0
    msr         BASEPRI, r0 // store OS_BASEPRI_MASK into BASEPRI register
    dsb // finish pending memory accesses 
    isb // synchronize instruction execution

    // get current task
    ldr         r3, =OS_Current // load OS_Current into r3
    ldr         r2, [r3] // load current task TCB
    cbz         r2, Pend_restore // if first task -> jump to Pend_restore

    mrs         r0, PSP // load current task stack pointer

    // save FPU context if needed
    tst         r14, #0x10 // check EXC_RETURN FP frame bit (0xFFFFFFED)
    it          eq // execute next instruction if bit is 0
    vstmdbeq    r0!, {s16-s31} // save FPU registers

    // save task context
    stmdb       r0!, {r4-r11, lr} // save R4-R11 and EXC_RETURN
    str         r0, [r2] // save stack pointer in TCB

    // check if stack overflow occured sp < stackBase
    ldr         r1, [r2, #OS_STACK_BASE_OFFSET] // load stack base
    cmp         r0, r1 // compare SP with stack base
    blo         OS_StackOverflowHandler // branch if SP is below stack base

Pend_restore:
    // switch current task
    ldr         r1, =OS_Next // load address of OS_Next
    ldr         r2, [r1] // load next task TCB
    str         r2, [r3] // OS_Current = OS_Next

    // restore task context
    ldr         r0, [r2] // load task stack pointer
    ldmia       r0!, {r4-r11, lr} // restore R4-R11 and EXC_RETURN

    // restore FPU context if needed
    tst         r14, #0x10 // check EXC_RETURN FP frame bit (0xFFFFFFED)
    it          eq // execute next instruction if bit is 0
    vldmiaeq    r0!, {s16-s31} // restore FPU registers

    msr         PSP, r0 // restore task stack pointer

    // exit critical section
    mov         r0, #0
    msr         BASEPRI, r0 // remove interrupt priority threshold

    bx          lr // return to task