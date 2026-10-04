#ifndef SHEDULER_H
#define SHEDULER_H

#include "task.h"
#include "stddef.h"

extern OSThread * volatile OS_Current;
extern OSThread * volatile OS_Next;

#endif