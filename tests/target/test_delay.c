#include "scheduler.h"
#include "task.h"
#include "port.h"
#include <stdint.h>

#define TEST_DELAY_TICKS    10
#define TEST_STACK_SIZE     128

typedef enum {
    TEST_RUNNING = 0,
    TEST_PASS,
    TEST_FAIL
    
} TestResult;

static OSThread testTask;

_Alignas(8) static uint32_t testStack[TEST_STACK_SIZE];

volatile TestResult testResult = TEST_RUNNING;

volatile uint32_t startTick = 0;
volatile uint32_t endTick = 0;
volatile uint32_t elapsedTicks = 0;

// add testResult, startTick, endTick and elapsedTicks to the debugger watch window

static void DelayTestTask(void)  {
    startTick = OS_GetTickCount();

    OS_Delay(TEST_DELAY_TICKS);

    endTick = OS_GetTickCount();
    elapsedTicks = endTick - startTick;

    if (elapsedTicks >= TEST_DELAY_TICKS) {
        testResult = TEST_PASS;
    } else {
        testResult = TEST_FAIL;
    }

    OS_Port_Breakpoint();

    for (;;) {
    }
}


void OS_Test_Delay_Run(void) // add testResult, startTick, endTick and elapsedTicks to the debugger watch window
{
    OS_Init();

    OS_Thread_Create(&testTask, DelayTestTask, 1, testStack, sizeof(testStack));

    OS_Start();

    for (;;) {
    }
}