#include "rtos_assert.h"
#include "stm32l4xx.h"

void OS_AssertFailed(const char *file, int line) {
    // store information about file and line where assert failed
    (void)file;
    (void)line;

    __disable_irq(); 
    __BKPT(0); // stop the debugger at this line

    while (1) { 

    }
}