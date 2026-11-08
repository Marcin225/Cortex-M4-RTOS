cmake_minimum_required(VERSION 3.22)

project(Cortex-M4-RTOS-Tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG master
)

FetchContent_MakeAvailable(unity)

enable_testing()

add_executable(rtos_tests
    unit/test_list.c
    unit/test_tick_overflow.c
    unit/test_main.c
    unit/test_scheduler.c

    support/rtos_assert_stub.c
    support/port_stub.c

    ../RTOS/kernel/scheduler.c
    ../RTOS/kernel/list.c
    ../RTOS/kernel/task.c
)

target_include_directories(rtos_tests PRIVATE
    ../RTOS/include
    ../RTOS/port/cortex_m4
)

target_link_libraries(rtos_tests PRIVATE
    unity
)

target_compile_definitions(rtos_tests PRIVATE
    OS_TEST
)

add_test(
    NAME rtos
    COMMAND rtos_tests
)