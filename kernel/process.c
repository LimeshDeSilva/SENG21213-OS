#include "process.h"
#include "vga.h"

extern void context_switch(uint32_t *old_esp, uint32_t new_esp);

static process_t process_table[MAX_PROCESSES];
static int current_process = 0;
static uint32_t next_pid = 1;

void process_init(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = PROCESS_UNUSED;
    }

    process_table[0].pid = next_pid++;
    process_table[0].state = PROCESS_RUNNING;
    int j = 0;
    char *init_name = "shell";
    while (init_name[j]) {
        process_table[0].name[j] = init_name[j];
        j++;
    }
    process_table[0].name[j] = '\0';
    current_process = 0;
}

int process_create(const char *name, void (*entry)(void)) {
    int slot = -1;
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCESS_UNUSED) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return -1;

    process_t *p = &process_table[slot];
    p->pid = next_pid++;
    p->state = PROCESS_READY;

    int i = 0;
    while (name[i] && i < 31) {
        p->name[i] = name[i];
        i++;
    }
    p->name[i] = '\0';

    uint32_t *sp = (uint32_t *)&p->stack[STACK_SIZE - 4];

    *(--sp) = (uint32_t)process_exit;
    *(--sp) = (uint32_t)entry;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;

    p->esp = (uint32_t)sp;
    return p->pid;
}

void process_yield(void) {
    int prev = current_process;
    int next = (current_process + 1) % MAX_PROCESSES;

    while (next != current_process) {
        if (process_table[next].state == PROCESS_READY) {
            break;
        }
        next = (next + 1) % MAX_PROCESSES;
    }

    if (next == current_process || process_table[next].state != PROCESS_READY) {
        return;
    }

    if (process_table[prev].state == PROCESS_RUNNING) {
        process_table[prev].state = PROCESS_READY;
    }
    process_table[next].state = PROCESS_RUNNING;
    current_process = next;

    context_switch(&process_table[prev].esp, process_table[next].esp);
}

void process_exit(void) {
    vga_puts("\n[Process ");
    vga_puts(process_table[current_process].name);
    vga_puts(" completed]\n");

    process_table[current_process].state = PROCESS_UNUSED;
    process_yield();
    while (1);
}

void process_list(void) {
    vga_puts("PID    STATE       NAME\n");
    vga_puts("---    -----       ----\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCESS_UNUSED) continue;

        vga_putchar('0' + process_table[i].pid);
        vga_puts("      ");

        switch (process_table[i].state) {
            case PROCESS_RUNNING: vga_puts("RUNNING     "); break;
            case PROCESS_READY:   vga_puts("READY       "); break;
            case PROCESS_BLOCKED: vga_puts("BLOCKED     "); break;
            default:              vga_puts("UNKNOWN     "); break;
        }

        vga_puts(process_table[i].name);
        vga_puts("\n");
    }
}
