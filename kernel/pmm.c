#include "pmm.h"
#include "vga.h"

static uint32_t page_bitmap[BITMAP_SIZE];
static uint32_t total_page_count = 0;
static uint32_t free_page_count = 0;

static inline void set_bit(uint32_t bit) {
    page_bitmap[bit / 32] |= (1 << (bit % 32));
}

static inline void clear_bit(uint32_t bit) {
    page_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

static inline int test_bit(uint32_t bit) {
    return (page_bitmap[bit / 32] & (1 << (bit % 32))) != 0;
}

void pmm_init(uint32_t mem_size) {
    total_page_count = mem_size / PAGE_SIZE;
    free_page_count = total_page_count;

    // Initialize all pages as free (0)
    for (uint32_t i = 0; i < BITMAP_SIZE; i++) {
        page_bitmap[i] = 0;
    }

    // Reserve the first 1MB of memory (BIOS, VGA, Kernel image)
    // 1MB = 256 pages (256 * 4096 = 1,048,576)
    uint32_t reserved_pages = (1024 * 1024) / PAGE_SIZE;
    for (uint32_t i = 0; i < reserved_pages; i++) {
        set_bit(i);
        free_page_count--;
    }
}

void *pmm_alloc_page(void) {
    for (uint32_t i = 0; i < total_page_count; i++) {
        if (!test_bit(i)) {
            set_bit(i);
            free_page_count--;
            return (void *)(i * PAGE_SIZE);
        }
    }
    return (void *)0; // Out of memory
}

void pmm_free_page(void *ptr) {
    uint32_t addr = (uint32_t)ptr;
    uint32_t page = addr / PAGE_SIZE;

    if (page < total_page_count && test_bit(page)) {
        clear_bit(page);
        free_page_count++;
    }
}

uint32_t pmm_get_free_pages(void) {
    return free_page_count;
}

uint32_t pmm_get_used_pages(void) {
    return total_page_count - free_page_count;
}
