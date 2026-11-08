#include "unity.h"
#include "list.h"
#include <stdint.h>

static OS_List list;
static OS_ListItem item1;
static OS_ListItem item2;
static OS_ListItem item3;

static void init_list_test(void) {
    OS_List_Init(&list);

    item1.next = NULL;
    item1.prev = NULL;
    item1.tickCount = 0;
    item1.owner = NULL;

    item2.next = NULL;
    item2.prev = NULL;
    item2.tickCount = 0;
    item2.owner = NULL;

    item3.next = NULL;
    item3.prev = NULL;
    item3.tickCount = 0;
    item3.owner = NULL;
}

void test_list_init_should_create_empty_list(void) {
    init_list_test();

    TEST_ASSERT_EQUAL_UINT32(0, list.numberOfItems);

    TEST_ASSERT_EQUAL_PTR(&list.endNode, list.currentNode);

    TEST_ASSERT_EQUAL_PTR(&list.endNode, list.endNode.next);
    TEST_ASSERT_EQUAL_PTR(&list.endNode, list.endNode.prev);

    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, list.endNode.tickCount);
}

void test_insert_end_should_add_first_element(void) {
    init_list_test();

    OS_List_InsertEnd(&list, &item1);

    TEST_ASSERT_EQUAL_UINT32(1, list.numberOfItems);

    TEST_ASSERT_EQUAL_PTR(&item1, list.endNode.next);
    TEST_ASSERT_EQUAL_PTR(&item1, list.endNode.prev);

    TEST_ASSERT_EQUAL_PTR(&list.endNode, item1.next);
    TEST_ASSERT_EQUAL_PTR(&list.endNode, item1.prev);
}

void test_insert_end_should_preserve_insertion_order(void) {
    init_list_test();

    OS_List_InsertEnd(&list, &item1);
    OS_List_InsertEnd(&list, &item2);
    OS_List_InsertEnd(&list, &item3);

    TEST_ASSERT_EQUAL_UINT32(3, list.numberOfItems);

    TEST_ASSERT_EQUAL_PTR(&item1, list.endNode.next);

    TEST_ASSERT_EQUAL_PTR(&item2, item1.next);
    TEST_ASSERT_EQUAL_PTR(&item1, item2.prev);

    TEST_ASSERT_EQUAL_PTR(&item3, item2.next);
    TEST_ASSERT_EQUAL_PTR(&item2, item3.prev);

    TEST_ASSERT_EQUAL_PTR(&list.endNode, item3.next);
    TEST_ASSERT_EQUAL_PTR(&item3, list.endNode.prev);
}

void test_insert_should_sort_elements_by_tick_count(void) {
    init_list_test();

    item1.tickCount = 30;
    item2.tickCount = 10;
    item3.tickCount = 20;

    OS_List_Insert(&list, &item1);
    OS_List_Insert(&list, &item2);
    OS_List_Insert(&list, &item3);

    TEST_ASSERT_EQUAL_UINT32(3, list.numberOfItems);

    TEST_ASSERT_EQUAL_PTR(&item2, list.endNode.next);
    TEST_ASSERT_EQUAL_PTR(&item3, item2.next);
    TEST_ASSERT_EQUAL_PTR(&item1, item3.next);
    TEST_ASSERT_EQUAL_PTR(&list.endNode, item1.next);

    TEST_ASSERT_EQUAL_PTR(&item1, list.endNode.prev);
}

void test_insert_should_place_equal_tick_after_existing_element(void) {
    init_list_test();

    item1.tickCount = 10;
    item2.tickCount = 10;

    OS_List_Insert(&list, &item1);
    OS_List_Insert(&list, &item2);

    TEST_ASSERT_EQUAL_UINT32(2, list.numberOfItems);

    TEST_ASSERT_EQUAL_PTR(&item1, list.endNode.next);
    TEST_ASSERT_EQUAL_PTR(&item2, item1.next);
    TEST_ASSERT_EQUAL_PTR(&list.endNode, item2.next);
}

void test_remove_should_remove_only_element(void) {
    init_list_test();

    OS_List_InsertEnd(&list, &item1);

    OS_List_Remove(&list, &item1);

    TEST_ASSERT_EQUAL_UINT32(0, list.numberOfItems);

    TEST_ASSERT_EQUAL_PTR(&list.endNode, list.endNode.next);
    TEST_ASSERT_EQUAL_PTR(&list.endNode, list.endNode.prev);

    TEST_ASSERT_NULL(item1.next);
    TEST_ASSERT_NULL(item1.prev);
}

void test_remove_should_remove_first_element(void) {
    init_list_test();

    OS_List_InsertEnd(&list, &item1);
    OS_List_InsertEnd(&list, &item2);
    OS_List_InsertEnd(&list, &item3);

    OS_List_Remove(&list, &item1);

    TEST_ASSERT_EQUAL_UINT32(2, list.numberOfItems);

    TEST_ASSERT_EQUAL_PTR(&item2, list.endNode.next);
    TEST_ASSERT_EQUAL_PTR(&list.endNode, item2.prev);

    TEST_ASSERT_NULL(item1.next);
    TEST_ASSERT_NULL(item1.prev);
}

void test_remove_should_remove_middle_element(void) {
    init_list_test();

    OS_List_InsertEnd(&list, &item1);
    OS_List_InsertEnd(&list, &item2);
    OS_List_InsertEnd(&list, &item3);

    OS_List_Remove(&list, &item2);

    TEST_ASSERT_EQUAL_UINT32(2, list.numberOfItems);

    TEST_ASSERT_EQUAL_PTR(&item3, item1.next);
    TEST_ASSERT_EQUAL_PTR(&item1, item3.prev);

    TEST_ASSERT_NULL(item2.next);
    TEST_ASSERT_NULL(item2.prev);
}

void test_remove_should_remove_last_element(void) {
    init_list_test();

    OS_List_InsertEnd(&list, &item1);
    OS_List_InsertEnd(&list, &item2);
    OS_List_InsertEnd(&list, &item3);

    OS_List_Remove(&list, &item3);

    TEST_ASSERT_EQUAL_UINT32(2, list.numberOfItems);

    TEST_ASSERT_EQUAL_PTR(&item2, list.endNode.prev);
    TEST_ASSERT_EQUAL_PTR(&list.endNode, item2.next);

    TEST_ASSERT_NULL(item3.next);
    TEST_ASSERT_NULL(item3.prev);
}

void test_remove_current_node_should_move_current_to_previous_node(void) {
    init_list_test();

    OS_List_InsertEnd(&list, &item1);
    OS_List_InsertEnd(&list, &item2);
    OS_List_InsertEnd(&list, &item3);

    list.currentNode = &item2;

    OS_List_Remove(&list, &item2);

    TEST_ASSERT_EQUAL_PTR(&item1, list.currentNode);
}