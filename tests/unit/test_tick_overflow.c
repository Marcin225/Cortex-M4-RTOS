#include "unity.h"
#include "scheduler.h"
#include "list.h"
#include <stdint.h>

void OS_Test_SetTickCount(uint32_t tickCount);
void OS_Test_InitTickLists(void);
OS_List *OS_Test_GetDelayedListPointer(void);
OS_List *OS_Test_GetOverflowListPointer(void);

static void init_tick_test(void) {
    OS_Test_SetTickCount(0);
    OS_Test_InitTickLists();
}

void test_tick_should_increment_normally(void) {
    init_tick_test();

    OS_Test_SetTickCount(100);

    OS_IncrementTick();

    TEST_ASSERT_EQUAL_UINT32(101, OS_GetTickCount());
}

void test_tick_should_reach_max_without_overflow(void) {
    init_tick_test();

    OS_List *delayedBefore = OS_Test_GetDelayedListPointer();
    OS_List *overflowBefore = OS_Test_GetOverflowListPointer();

    OS_Test_SetTickCount(UINT32_MAX - 1);

    OS_IncrementTick();

    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, OS_GetTickCount());

    TEST_ASSERT_EQUAL_PTR(
        delayedBefore,
        OS_Test_GetDelayedListPointer()
    );

    TEST_ASSERT_EQUAL_PTR(
        overflowBefore,
        OS_Test_GetOverflowListPointer()
    );
}

void test_tick_should_wrap_from_max_to_zero(void) {
    init_tick_test();

    OS_Test_SetTickCount(UINT32_MAX);

    OS_IncrementTick();

    TEST_ASSERT_EQUAL_UINT32(0, OS_GetTickCount());
}

void test_tick_overflow_should_swap_delayed_lists(void) {
    init_tick_test();

    OS_List *delayedBefore = OS_Test_GetDelayedListPointer();
    OS_List *overflowBefore = OS_Test_GetOverflowListPointer();

    OS_Test_SetTickCount(UINT32_MAX);

    OS_IncrementTick();

    TEST_ASSERT_EQUAL_PTR(
        overflowBefore,
        OS_Test_GetDelayedListPointer()
    );

    TEST_ASSERT_EQUAL_PTR(
        delayedBefore,
        OS_Test_GetOverflowListPointer()
    );
}