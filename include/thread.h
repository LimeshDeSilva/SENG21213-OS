#ifndef THREAD_H
#define THREAD_H

#include "types.h"

#define MAX_THREADS     8
#define THREAD_STACK_SZ 4096

typedef enum {
    THREAD_UNUSED = 0,
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_TERMINATED
} thread_state_t;

typedef struct thread {
    uint32_t tid;
    char name[32];
    thread_state_t state;
    uint32_t esp;
    uint8_t stack[THREAD_STACK_SZ];
} thread_t;

void thread_init(void);
int  thread_create(const char *name, void (*entry)(void));
void thread_yield(void);
void thread_exit(void);
void thread_list(void);

#endif
