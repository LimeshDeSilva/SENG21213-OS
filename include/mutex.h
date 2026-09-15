#ifndef MUTEX_H
#define MUTEX_H

#include "types.h"

typedef struct mutex {
    volatile uint32_t locked;
    int owner_tid;
} mutex_t;

void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);

#endif
