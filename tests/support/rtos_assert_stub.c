#include "rtos_assert.h"
#include <stdlib.h>

void OS_AssertFailed(const char *file, int line) {
    // store information about file and line where assert failed
    (void)file;
    (void)line;

    abort();
}