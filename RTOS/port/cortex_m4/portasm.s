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
    mov     r0, #OS_BASEPRI_MASK // move OS_BASEPRI_MASK into r0
    msr     BASEPRI, r0 // store OS_BASEPRI_MASK into BASEPRI register
    dsb // finish pending memory accesses 
    isb // synchronize instruction execution

    // if (OS_Current != (OSThread *)NULL) {
    ldr     r3, =OS_Current // load OS_Current into r3
    ldr     r2, [r3] // dereference value
    cbz     r2, Pend_restore // if first task -> jump to Pend_restore

    mrs     r0, PSP // load current thread stack pointer
    stmdb   r0!, {r4-r11} // push registers r4-r11 on stack 

    // OS_Current->sp = sp;
    str     r0, [r2] // save PSP pointer

Pend_restore:
    // OS_Current = OS_Next;
    ldr     r1, =OS_Next // load OS_Next into r1
    ldr     r2, [r1] // load next task TCB into r2
    str     r2, [r3] // save next task TCB

    // sp = OS_Current->sp;
    ldr     r0, [r2] // load current task sp into r0
    ldmia   r0!, {r4-r11} // pop registers r4-r11 on stack
    msr     PSP, r0 // store current task sp into PSP register

    // exit critical section
    mov     r0, #0
    msr     BASEPRI, r0 // remove interrupt priority threshold

    // ***************************************************************** optimization needed *****************************************
    ldr     lr, =0xFFFFFFFD // set EXC_RETURN to Thread mode with PSP
    bx      lr // return