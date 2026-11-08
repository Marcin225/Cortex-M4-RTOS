#include "unity.h"
#include "scheduler.h"
#include "task.h"
#include <stdint.h>

#define TEST_STACK_SIZE 128

static OSThread task1;
static OSThread task2;
static OSThread task3;

static uint32_t stack1[TEST_STACK_SIZE];
static uint32_t stack2[TEST_STACK_SIZE];
static uint32_t stack3[TEST_STACK_SIZE];

static void DummyTask(void) {
}

static void init_scheduler_test(void) {
    OS_Init();
}

// *********** Priority tests ***********

void test_scheduler_should_select_highest_priority_thread(void) {
    init_scheduler_test();

    OS_Thread_Create(&task1, DummyTask, 1, stack1, sizeof(stack1));
    OS_Thread_Create(&task2, DummyTask, 7, stack2, sizeof(stack2));
    OS_Thread_Create(&task3, DummyTask, 3, stack3, sizeof(stack3));

    OS_ChooseNextThread();

    TEST_ASSERT_EQUAL_PTR(&task2, OS_Next);
}

void test_scheduler_should_not_select_lower_priority_thread_while_higher_is_ready(void) {
    init_scheduler_test();

    OS_Thread_Create(&task1, DummyTask, 2, stack1, sizeof(stack1));
    OS_Thread_Create(&task2, DummyTask, 5, stack2, sizeof(stack2));

    for (int i = 0; i < 5; i++) {
        OS_ChooseNextThread();

        TEST_ASSERT_EQUAL_PTR(&task2, OS_Next);
    }
}

// *********** Round robin tests ***********

void test_scheduler_should_round_robin_threads_with_same_priority(void) {
    init_scheduler_test();

    OS_Thread_Create(&task1, DummyTask, 5, stack1, sizeof(stack1));
    OS_Thread_Create(&task2, DummyTask, 5, stack2, sizeof(stack2));
    OS_Thread_Create(&task3, DummyTask, 5, stack3, sizeof(stack3));

    OS_ChooseNextThread();
    TEST_ASSERT_EQUAL_PTR(&task1, OS_Next);

    OS_ChooseNextThread();
    TEST_ASSERT_EQUAL_PTR(&task2, OS_Next);

    OS_ChooseNextThread();
    TEST_ASSERT_EQUAL_PTR(&task3, OS_Next);

    OS_ChooseNextThread();
    TEST_ASSERT_EQUAL_PTR(&task1, OS_Next);
}

// *********** Delay tests ***********

void test_delay_should_not_wake_thread_before_requested_ticks(void) {
    init_scheduler_test();

    const uint32_t delayTicks = 10;

    OS_Thread_Create(&task1, DummyTask, 5, stack1, sizeof(stack1));

    OS_ChooseNextThread();

    OS_Current = OS_Next;

    uint32_t startTick = OS_GetTickCount();

    OS_Delay(delayTicks);

    // check every tick before requested delay
    for (uint32_t i = 0; i < delayTicks - 1; i++) {
        OS_IncrementTick();
        OS_ChooseNextThread();

        TEST_ASSERT_NOT_EQUAL(&task1, OS_Next);
    }

    OS_IncrementTick();
    OS_ChooseNextThread();

    uint32_t elapsedTicks = OS_GetTickCount() - startTick;

    TEST_ASSERT_TRUE(elapsedTicks >= delayTicks);
    TEST_ASSERT_EQUAL_PTR(&task1, OS_Next);
}