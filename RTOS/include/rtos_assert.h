#ifndef RTOS_ASSERT_H
#define RTOS_ASSERT_H

void OS_AssertFailed(const char *file, int line);

#define OS_ASSERT(condition)    condition ? (void)0 : (OS_AssertFailed(__FILE__, __LINE__));

#endif