#include "rtos_assert.h"
#include "port.h"

void OS_AssertFailed(const char *file, int line) {
    // store information about file and line where assert failed
    (void)file;
    (void)line;
    
    OS_Port_DisableInterrupts();
    OS_Port_Breakpoint(); // stop the debugger at this line

    while (1) { 

    }
}