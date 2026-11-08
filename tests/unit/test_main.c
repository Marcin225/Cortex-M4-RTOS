#include "unity.h"

// *********** LIST TESTS ***********

void test_list_init_should_create_empty_list(void);

void test_insert_end_should_add_first_element(void);
void test_insert_end_should_preserve_insertion_order(void);

void test_insert_should_sort_elements_by_tick_count(void);
void test_insert_should_place_equal_tick_after_existing_element(void);

void test_remove_should_remove_only_element(void);
void test_remove_should_remove_first_element(void);
void test_remove_should_remove_middle_element(void);
void test_remove_should_remove_last_element(void);
void test_remove_current_node_should_move_current_to_previous_node(void);

// *********** TICK TESTS ***********

void test_tick_should_increment_normally(void);
void test_tick_should_reach_max_without_overflow(void);
void test_tick_should_wrap_from_max_to_zero(void);
void test_tick_overflow_should_swap_delayed_lists(void);

// *********** SCHEDULER TESTS ***********

void test_scheduler_should_select_highest_priority_thread(void);
void test_scheduler_should_not_select_lower_priority_thread_while_higher_is_ready(void);
void test_scheduler_should_round_robin_threads_with_same_priority(void);
void test_delay_should_not_wake_thread_before_requested_ticks(void);

void setUp(void) {
    
}

void tearDown(void) {
}

int main(void) {
    UNITY_BEGIN();

    // *********** LIST TESTS ***********

    RUN_TEST(test_list_init_should_create_empty_list);

    RUN_TEST(test_insert_end_should_add_first_element);
    RUN_TEST(test_insert_end_should_preserve_insertion_order);

    RUN_TEST(test_insert_should_sort_elements_by_tick_count);
    RUN_TEST(test_insert_should_place_equal_tick_after_existing_element);

    RUN_TEST(test_remove_should_remove_only_element);
    RUN_TEST(test_remove_should_remove_first_element);
    RUN_TEST(test_remove_should_remove_middle_element);
    RUN_TEST(test_remove_should_remove_last_element);
    RUN_TEST(test_remove_current_node_should_move_current_to_previous_node);

    // *********** TICK TESTS ***********

    RUN_TEST(test_tick_should_increment_normally);
    RUN_TEST(test_tick_should_reach_max_without_overflow);
    RUN_TEST(test_tick_should_wrap_from_max_to_zero);
    RUN_TEST(test_tick_overflow_should_swap_delayed_lists);

    // *********** SCHEDULER TESTS ***********

    RUN_TEST(test_scheduler_should_select_highest_priority_thread);
    RUN_TEST(test_scheduler_should_not_select_lower_priority_thread_while_higher_is_ready);
    RUN_TEST(test_scheduler_should_round_robin_threads_with_same_priority);
    RUN_TEST(test_delay_should_not_wake_thread_before_requested_ticks);

    return UNITY_END();
}