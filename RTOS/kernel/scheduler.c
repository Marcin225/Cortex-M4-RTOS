#include "scheduler.h"
#include "task.h"

OSThread * volatile OS_Current = NULL;
OSThread * volatile OS_Next = NULL;

OSThread *threadPointerList[MAX_THREADS];

uint8_t threadNumber = 0;
uint8_t threadIdx = 0;

void OS_ChooseNextThread() {
    if (threadIdx >= threadNumber) {
        threadIdx = 0;
    }
    OS_Next = threadPointerList[threadIdx++];
}