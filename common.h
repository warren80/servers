#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#define BINDER      0
#define WORKER      1
#define METRIC      2
#define KEY         (996687)





void newThread(void *role);
void blockSignals();
PWORKERSTRUCT setupSemaphoreStruct();






#endif
