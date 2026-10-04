#include "scheduler.h"

OSThread * volatile OS_Current = NULL;
OSThread * volatile OS_Next = NULL;