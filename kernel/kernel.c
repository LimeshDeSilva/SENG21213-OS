/* =============================================================================
 * SENG21213-OS :: Main Kernel  (Stage 0 – Foundations)
 * File   : kernel/kernel.c
 *
 * PURPOSE
 *   This is the heart of your operating system. Right now it:
 *     1. Initialises VGA text-mode display
 *     2. Initialises the keyboard driver
 *     3. Prints a splash screen
 *     4. Runs a minimal interactive shell ("ksh")
 *
 * ASSIGNMENT MILESTONES  (what YOU will add in later lectures)
 *   Lecture  9  – Process Management  →  process.h / process.c / scheduler.c
 *   Lecture 10  – Threads             →  thread.h  / thread.c
 *   Lecture 11  – Memory Management   →  pmm.h     / pmm.c / vmm.c
 *   Lecture 12  – File System         →  fs.h      / fs.c
 *
 * CODING CONVENTION
 *   - Prefix kernel-internal functions with k_ (e.g. k_strcmp)
 *   - All driver APIs live in their own .h/.c pair
 *   - NEVER call malloc – use the PMM you build in Lecture 11
 * ============================================================================*/

#include "vga.h"
#include "keyboard.h"
#include "../include/types.h"
#include "process.h"
#include "thread.h"
#include "mutex.h"
#include "pmm.h"
#include "fs.h"
/* ---------------------------------------------------------------------------
 * Forward declarations of shell commands
 * --------------------------------------------------------------------------*/
static void cmd_help(void);
static void cmd_clear(void);
static void cmd_about(void);
static void cmd_echo(const char *args);
static void cmd_mem(void);
static void cmd_ps(void);
static void cmd_spawn(void);
static void cmd_yield(void);

/* ---------------------------------------------------------------------------
 * Utility: minimal string helpers (no libc in a freestanding kernel!)
 * --------------------------------------------------------------------------*/
static int k_strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (uint8_t)*a - (uint8_t)*b;
}

static int k_strncmp(const char *a, const char *b, size_t n) {
    while (n-- && *a && (*a == *b)) { a++; b++; }
    return n == (size_t)-1 ? 0 : (uint8_t)*a - (uint8_t)*b;
}

static size_t k_strlen(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

/* Skip leading spaces */
static const char *k_ltrim(const char *s) {
    while (*s == ' ') s++;
    return s;
}

/* ---------------------------------------------------------------------------
 * Splash Screen
 * --------------------------------------------------------------------------*/
static void print_splash(void) {
    vga_clear(VGA_BLACK);

    /* Top banner box */
    vga_draw_box(0, 0, 7, 80, VGA_LIGHT_MAGENTA);

    vga_set_cursor(1, 2);
    vga_puts_color("  SENG21213-OS  |  Computer Architecture & Operating Systems",
                   VGA_YELLOW, VGA_BLACK);

    vga_set_cursor(2, 2);
    vga_puts_color("  Stage 0: Kernel Foundations", VGA_LIGHT_CYAN, VGA_BLACK);

    vga_set_cursor(3, 2);
    vga_puts_color("  Faculty of Engineering – Department of Software Engineering",
                   VGA_LIGHT_GREY, VGA_BLACK);

    vga_set_cursor(4, 2);
    vga_puts_color("  Built by students, for students.  Type 'help' to begin.",
                   VGA_LIGHT_GREEN, VGA_BLACK);

    vga_set_cursor(5, 2);
    vga_puts_color("  CPU: i686 (32-bit Protected Mode)  |  Display: VGA 80x25",
                   VGA_DARK_GREY, VGA_BLACK);

    vga_set_cursor(8, 0);
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("  Welcome! This kernel was compiled from source and booted entirely\n");
    vga_puts("  from bare metal. There is no Linux or Windows underneath – only\n");
    vga_puts("  the code you and your team write.\n");
    vga_puts("\n");
    vga_puts("  Assignment milestones to implement:\n");
    vga_puts_color("    [L09] ", VGA_YELLOW, VGA_BLACK);
    vga_puts("Process Management  – PCB, ready queue, round-robin scheduler\n");
    vga_puts_color("    [L10] ", VGA_YELLOW, VGA_BLACK);
    vga_puts("Threads & Sync      – kernel threads, mutex, semaphore\n");
    vga_puts_color("    [L11] ", VGA_YELLOW, VGA_BLACK);
    vga_puts("Memory Management   – physical page allocator, virtual memory\n");
    vga_puts_color("    [L12] ", VGA_YELLOW, VGA_BLACK);
    vga_puts("File System         – RAM disk, FAT-like directory structure\n");
    vga_puts("\n");
}

/* ---------------------------------------------------------------------------
 * Shell command implementations
 * --------------------------------------------------------------------------*/
static void cmd_help(void) {
    vga_puts_color("\n  SENG21213-OS Shell Commands\n", VGA_YELLOW, VGA_BLACK);
    vga_puts("  ─────────────────────────────────────────────\n");
    vga_puts("  help    – Show this help message\n");
    vga_puts("  clear   – Clear the screen\n");
    vga_puts("  about   – About this OS and course\n");
    vga_puts("  echo    – Echo text to screen\n");
    vga_puts("  mem     – Memory map (stub)\n");
    vga_puts_color("\n  Milestones (to implement):\n", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("  ps      – [L09] List processes\n");
    vga_puts("  kill    – [L09] Terminate a process\n");
    vga_puts("  threads – [L10] List kernel threads\n");
    vga_puts("  free    – [L11] Show free memory\n");
    vga_puts("  ls      – [L12] List files\n");
    vga_puts("  cat     – [L12] Print file contents\n\n");
}

static void cmd_clear(void) {
    vga_clear(VGA_BLACK);
}

static void cmd_about(void) {
    vga_puts_color("\n  About SENG21213-OS\n", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("  ─────────────────────────────────────────────\n");
    vga_puts("  Architecture : x86 (i686), 32-bit Protected Mode\n");
    vga_puts("  Bootloader   : Custom MBR (NASM)\n");
    vga_puts("  Kernel       : Freestanding C (GCC, no libc)\n");
    vga_puts("  VM Target    : QEMU (qemu-system-i386)\n");
    vga_puts("  Course       : SENG 21213 – Sem 2\n");
    vga_puts("  Reference    : Stallings, OS: Internals & Design Principles\n\n");
}

static void cmd_echo(const char *args) {
    vga_puts("  ");
    vga_puts(args);
    vga_puts("\n");
}

static void cmd_mem(void) {
    /* Stage 0 stub – students implement the real PMM in Lecture 11 */
    vga_puts_color("\n  Memory Map (stub – implement PMM in Lecture 11)\n",
                   VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("  ─────────────────────────────────────────────\n");
    vga_puts("  0x00000000 – 0x000FFFFF  :  First 1 MB (reserved/BIOS)\n");
    vga_puts("  0x00100000 – 0x00EFFFFF  :  Extended memory (usable ~14 MB)\n");
    vga_puts("  0x00F00000 – 0x00FFFFFF  :  BIOS / ROM area\n");
    vga_puts("  0xB8000    – 0xBFFFF     :  VGA frame buffer\n");
    vga_puts_color("\n  TODO: Use BIOS int 0x15, EAX=0xE820 to get real memory map\n\n",
                   VGA_YELLOW, VGA_BLACK);
}

/* ---------------------------------------------------------------------------
 * Shell process
 * --------------------------------------------------------------------------*/
static char  shell_buf[256];
static char  prompt[] = "\n  ksh> ";

static void worker_task(void) {
    vga_puts("\n  [Worker Task] Started. Doing work step 1...\n");
    process_yield();
    vga_puts("\n  [Worker Task] Resumed. Doing work step 2...\n");
    process_yield();
    vga_puts("\n  [Worker Task] Work finished. Exiting.\n");
}

static void cmd_ps(void) {
    vga_puts("\n");
    process_list();
}

static void cmd_spawn(void) {
    int pid = process_create("worker", worker_task);
    if (pid >= 0) {
        vga_puts("\n  [Spawned process 'worker' with PID ");
        vga_putchar('0' + pid);
        vga_puts("]\n");
    } else {
        vga_puts("\n  [Failed: process table full]\n");
    }
}

static void cmd_yield(void) {
    vga_puts("\n  [Yielding CPU to next READY task...]\n");
    process_yield();
}

static mutex_t test_mutex;
static int shared_counter = 0;

static void worker_thread(void) {
    vga_puts("\n  [Thread Worker] Trying to acquire mutex...\n");
    mutex_lock(&test_mutex);
    vga_puts("  [Thread Worker] Mutex acquired! Modifying shared resource.\n");
    shared_counter += 10;
    mutex_unlock(&test_mutex);
    vga_puts("  [Thread Worker] Mutex released. Exiting.\n");
}

static void cmd_threads(void) {
    vga_puts("\n");
    thread_list();
}

static void cmd_mutex_test(void) {
    vga_puts("\n  [Mutex Test] Initializing mutex and spawning worker thread...\n");
    mutex_init(&test_mutex);
    shared_counter = 5;
    thread_create("m_worker", worker_thread);

    vga_puts("  [Main Context] Yielding to worker thread...\n");
    thread_yield();

    vga_puts("  [Main Context] Resumed. Shared counter value: ");
    vga_putchar('0' + (shared_counter / 10));
    vga_putchar('0' + (shared_counter % 10));
    vga_puts("\n");
}

static void *test_allocated_page = 0;

static void cmd_free(void) {
    uint32_t free_pg = pmm_get_free_pages();
    uint32_t used_pg = pmm_get_used_pages();
    uint32_t free_kb = free_pg * 4;
    uint32_t used_kb = used_pg * 4;

    vga_puts("\n  --- Physical Memory Statistics ---\n");
    vga_puts("  Page Size : 4 KB\n");
    vga_puts("  Total RAM : 32 MB (8192 pages)\n");
    vga_printf("  Used Pages: %d (%d KB)\n", used_pg, used_kb);
    vga_printf("  Free Pages: %d (%d KB)\n", free_pg, free_kb);
}

static void cmd_alloc(void) {
    test_allocated_page = pmm_alloc_page();
    if (test_allocated_page) {
        vga_printf("\n  [PMM] Allocated 4KB physical frame at: 0x%x\n", (uint32_t)test_allocated_page);
    } else {
        vga_puts("\n  [PMM] Allocation failed: out of physical memory!\n");
    }
}

static void cmd_dealloc(void) {
    if (test_allocated_page) {
        pmm_free_page(test_allocated_page);
        vga_printf("\n  [PMM] Freed page at: 0x%x\n", (uint32_t)test_allocated_page);
        test_allocated_page = 0;
    } else {
        vga_puts("\n  [PMM] No page currently allocated to free.\n");
    }
}

static void cmd_ls(void) {
    vga_puts("\n");
    fs_list();
}

static void cmd_cat(const char *filename) {
    static char file_buf[1024];
    if (k_strlen(filename) == 0) {
        vga_puts("\n  Usage: cat <filename>\n");
        return;
    }

    int bytes = fs_read(filename, file_buf, sizeof(file_buf));
    if (bytes >= 0) {
        vga_puts("\n");
        vga_puts(file_buf);
    } else {
        vga_printf("\n  [FS] File not found: %s\n", filename);
    }
}

static void cmd_write(const char *arg) {
    if (k_strlen(arg) == 0) {
        vga_puts("\n  Usage: write <filename> <text>\n");
        return;
    }

    char name[32];
    int i = 0;
    while (arg[i] && arg[i] != ' ' && i < 31) {
        name[i] = arg[i];
        i++;
    }
    name[i] = '\0';

    while (arg[i] == ' ') i++;

    const char *data = &arg[i];
    uint32_t len = k_strlen(data);

    if (fs_write(name, data, len) >= 0) {
        vga_printf("\n  [FS] Successfully wrote %d bytes to %s\n", len, name);
    } else {
        vga_puts("\n  [FS] Write failed: disk full!\n");
    }
}

static void shell_run(void) {
    vga_puts_color("\n  Kernel Shell ready. Type 'help' for commands.\n",
                   VGA_LIGHT_GREEN, VGA_BLACK);

    while (true) {
        vga_puts_color(prompt, VGA_LIGHT_GREEN, VGA_BLACK);
        kb_readline(shell_buf, sizeof(shell_buf));

        /* Trim leading whitespace */
        const char *cmd = k_ltrim(shell_buf);
        if (k_strlen(cmd) == 0) continue;

        /* Dispatch */
        if (k_strcmp(cmd, "help")  == 0) { cmd_help();  continue; }
        if (k_strcmp(cmd, "clear") == 0) { cmd_clear(); continue; }
        if (k_strcmp(cmd, "about") == 0) { cmd_about(); continue; }
        if (k_strcmp(cmd, "mem")   == 0) { cmd_mem();   continue; }
	if (k_strcmp(cmd, "ps")    == 0) { cmd_ps();    continue; }
        if (k_strcmp(cmd, "spawn") == 0) { cmd_spawn(); continue; }
        if (k_strcmp(cmd, "yield") == 0) { cmd_yield(); continue; }
	if (k_strcmp(cmd, "threads") == 0) { cmd_threads();    continue; }
        if (k_strcmp(cmd, "mutex")   == 0) { cmd_mutex_test(); continue; }
	if (k_strcmp(cmd, "free")    == 0) { cmd_free();    continue; }
        if (k_strcmp(cmd, "alloc")   == 0) { cmd_alloc();   continue; }
        if (k_strcmp(cmd, "dealloc") == 0) { cmd_dealloc(); continue; }
	if (k_strcmp(cmd, "ls") == 0) { cmd_ls(); continue; }

        if (k_strncmp(cmd, "cat ", 4) == 0) {
            cmd_cat(k_ltrim(cmd + 4));
            continue;
        }

        if (k_strncmp(cmd, "write ", 6) == 0) {
            cmd_write(k_ltrim(cmd + 6));
            continue;
        }

        if (k_strncmp(cmd, "echo ", 5) == 0) {
            cmd_echo(k_ltrim(cmd + 5));
            continue;
        }

        /* Milestone stubs */
        if (k_strcmp(cmd, "kill") == 0) {    
            vga_puts_color("  [TODO] This command is not yet implemented.\n",
                           VGA_YELLOW, VGA_BLACK);
            vga_puts("  Implement it as part of your lecture assignment.\n");
            continue;
        }

        vga_puts_color("  Unknown command: ", VGA_LIGHT_RED, VGA_BLACK);
        vga_puts(cmd);
        vga_puts("\n  Type 'help' for a list of commands.\n");
    }
}

/* ---------------------------------------------------------------------------
 * Kernel entry point – called from kernel_entry.asm
 * --------------------------------------------------------------------------*/
void kernel_main(void) {
    vga_init();
    kb_init();
    pmm_init(32 * 1024 * 1024);
    fs_init();
    process_init();
    thread_init();
    print_splash();
    shell_run();

    /* Should never reach here */
    __asm__ __volatile__("hlt");
}
