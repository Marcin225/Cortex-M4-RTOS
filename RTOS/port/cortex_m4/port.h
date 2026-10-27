#ifndef PORT_H
#define PORT_H

#include <stdint.h>
#include "stddef.h"
#include "task.h"

uint32_t *OS_Port_InitStack(OSThreadHandler threadHandler, void *stkMem, size_t stkSize);
void OS_Port_Init(void);
void OS_Port_EnableTick(void);
void OS_Port_RequestContextSwitch(void);
uint32_t OS_Port_EnterCritical(void);
void OS_Port_ExitCritical(uint32_t prevBASEPRI);

#endif