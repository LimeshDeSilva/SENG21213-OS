#include "thread.h"
#include "vga.h"

extern void context_switch(uint32_t *old_esp, uint32_t new_esp);

static thread_t thread_table[MAX_THREADS];
static int current_thread = 0;
static uint32_t next_tid = 1;

void thread_init(void) {
    for (int i = 0; i < MAX_THREADS; i++) {
        thread_table[i].tid = 0;
        thread_table[i].state = THREAD_UNUSED;
    }

    // Thread 0 represents the main execution context
    thread_table[0].tid = next_tid++;
    thread_table[0].state = THREAD_RUNNING;
    
    char *name = "main_kthread";
    int j = 0;
    while (name[j]) {
        thread_table[0].name[j] = name[j];
        j++;
    }
    thread_table[0].name[j] = '\0';
    current_thread = 0;
}

int thread_create(const char *name, void (*entry)(void)) {
    int slot = -1;
    for (int i = 1; i < MAX_THREADS; i++) {
        if (thread_table[i].state == THREAD_UNUSED) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return -1;

    thread_t *t = &thread_table[slot];
    t->tid = next_tid++;
    t->state = THREAD_READY;

    int i = 0;
    while (name[i] && i < 31) {
        t->name[i] = name[i];
        i++;
    }
    t->name[i] = '\0';

    uint32_t *sp = (uint32_t *)&t->stack[THREAD_STACK_SZ - 4];
    *(--sp) = (uint32_t)thread_exit;
    *(--sp) = (uint32_t)entry;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;

    t->esp = (uint32_t)sp;
    return t->tid;
}

void thread_yield(void) {
    int prev = current_thread;
    int next = (current_thread + 1) % MAX_THREADS;

    while (next != current_thread) {
        if (thread_table[next].state == THREAD_READY) {
            break;
        }
        next = (next + 1) % MAX_THREADS;
    }

    if (next == current_thread || thread_table[next].state != THREAD_READY) {
        return;
    }

    if (thread_table[prev].state == THREAD_RUNNING) {
        thread_table[prev].state = THREAD_READY;
    }
    thread_table[next].state = THREAD_RUNNING;
    current_thread = next;

    context_switch(&thread_table[prev].esp, thread_table[next].esp);
}

void thread_exit(void) {
    vga_puts("\n  [Thread ");
    vga_puts(thread_table[current_thread].name);
    vga_puts(" completed]\n");

    thread_table[current_thread].state = THREAD_UNUSED;
    thread_yield();
    while (1);
}

void thread_list(void) {
    vga_puts("TID    STATE       NAME\n");
    vga_puts("---    -----       ----\n");
    for (int i = 0; i < MAX_THREADS; i++) {
        if (thread_table[i].state == THREAD_UNUSED) continue;

        vga_putchar('0' + thread_table[i].tid);
        vga_puts("      ");

        switch (thread_table[i].state) {
            case THREAD_RUNNING: vga_puts("RUNNING     "); break;
            case THREAD_READY:   vga_puts("READY       "); break;
            case THREAD_BLOCKED: vga_puts("BLOCKED     "); break;
            default:             vga_puts("UNKNOWN     "); break;
        }

        vga_puts(thread_table[i].name);
        vga_puts("\n");
    }
}
