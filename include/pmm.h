#ifndef PMM_H
#define PMM_H

#include "types.h"

#define PAGE_SIZE       4096
#define TOTAL_MEMORY    (32 * 1024 * 1024)   // 32MB QEMU memory
#define TOTAL_PAGES     (TOTAL_MEMORY / PAGE_SIZE)
#define BITMAP_SIZE     (TOTAL_PAGES / 32)

void     pmm_init(uint32_t mem_size);
void    *pmm_alloc_page(void);
void     pmm_free_page(void *ptr);
uint32_t pmm_get_free_pages(void);
uint32_t pmm_get_used_pages(void);

#endif
