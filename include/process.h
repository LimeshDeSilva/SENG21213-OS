#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

#define MAX_PROCESSES   8
#define STACK_SIZE      4096

typedef enum {
    PROCESS_UNUSED = 0,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} process_state_t;

typedef struct process {
    uint32_t pid;
    char name[32];
    process_state_t state;
    uint32_t esp;                  // Saved stack pointer
    uint8_t stack[STACK_SIZE];     // Dedicated process stack
    uint32_t sleep_ticks;
} process_t;

void process_init(void);
int  process_create(const char *name, void (*entry)(void));
void process_yield(void);
void process_exit(void);
void process_list(void);

#endif
