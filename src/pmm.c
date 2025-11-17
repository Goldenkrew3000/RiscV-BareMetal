/*
 * Hojuix Developer Pre-Alpha riscv64 PMM
 */

#include <stddef.h>
#include <stdint.h>
#include <pmm.h>
#include <kprintf.h>

uint8_t* bitmap = (void*)0;
uint64_t pmm_used_pages = 0;
uint64_t pmm_free_pages = 0;
uint64_t pmm_total_pages = 0;

// Page rounding functions extracted from Xv6
#define PGSIZE 0x1000
#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))

extern uint64_t kern_end; // Defined in linker
uint64_t mem_bottom = 0;
uint64_t mem_top = 0x8f000000;

void pmm_init() {
    printf("[PMM] Init.\n");

    // Calculate the page-aligned address of the kernel end
    uint64_t kern_end_addr = (uint64_t)&kern_end;
    kern_end_addr = PGROUNDUP(kern_end_addr);
    printf("[PMM] Kernel ends at 0x%llx\n", kern_end_addr);

    // Calculate total possible page count
    uint32_t pmm_page_count = (mem_top - kern_end_addr) / 4096;
    printf("[PMM] Total page count: %ld pages\n", pmm_page_count);

    // Calculate the amount of memory required to hold a bitmap
    uint32_t pmm_bitmap_size = (pmm_page_count + 8 - 1) / 8;
    printf("[PMM] Bitmap size: %ld bytes\n", pmm_bitmap_size);
    uint32_t pmm_bitmap_page_size = (pmm_bitmap_size + 0x1000 - 1) / 0x1000; // Works but check later
    printf("[PMM] Bitmap size: %ld pages\n", pmm_bitmap_page_size);

    // Calculate memory addresses
    uint64_t bitmap_addr = kern_end_addr;
    mem_bottom = kern_end_addr + (pmm_bitmap_page_size * 0x1000);
    printf("[PMM] Bitmap Location: 0x%llx - %lld bytes / %lld pages\n", bitmap_addr, pmm_bitmap_size, pmm_bitmap_page_size);
    printf("[PMM] Usable memory: 0x%llx - 0x%llx\n", mem_bottom, mem_top);

    // Assign bitmap address, and set every page to used
    bitmap = (void*)bitmap_addr;
    // Poor man's memset(tm)
    for (uint64_t i = 0; i < (pmm_bitmap_page_size * 0x1000); i++) {
        bitmap[i] = 0xFF;
    }
    pmm_used_pages = pmm_page_count;

    pmm_total_pages = pmm_page_count;
    uint64_t page = 2; // First 2 pages are reserved
    for (uint64_t i = 0; i < (pmm_page_count - pmm_bitmap_page_size); i++) {
        pmm_set_free(page + i);
    }

    printf("[PMM] Init'd.\n");
}

void pmm_set_used(uint64_t page) {
    uint64_t byte = page / 8;
    uint64_t bit = page % 8;
    bitmap[byte] |= (1 << bit);
    pmm_used_pages++;
    pmm_free_pages--;
}

void pmm_set_free(uint64_t page) {
    uint64_t byte = page / 8;
    uint64_t bit = page % 8;
    bitmap[byte] &= ~(1 << bit);
    pmm_used_pages--;
    pmm_free_pages++;
}

uint8_t pmm_is_page_used(uint64_t page) {
    uint64_t byte = page / 8;
    uint64_t bit = page % 8;
    return (bitmap[byte] & (1 << bit)) >> bit;
}

uint64_t pmm_find_free_pages(uint64_t size) {
    uint64_t needed_pages = size;
    uint64_t found_pages = 0;
    uint64_t current_page = 0;

    for (uint64_t i = 0; i < pmm_total_pages; i++) {
        if (!pmm_is_page_used(i)) {
            if (found_pages == 0) {
                current_page = i;
            }
            found_pages++;
        } else {
            found_pages = 0;
        }

        if (found_pages >= needed_pages) {
            return current_page;
        }
    }

    printf("[PMM] FATAL: Could not find free memory.\n");
}

uint64_t pmm_kalloc(int pages) {
    uint64_t page = pmm_find_free_pages(pages);

    // Set pages as used
    for (uint64_t i = 0; i < pages; i++) {
        pmm_set_used(page + i);
    }

    // Send back physical address
    // NOTE: mem_bottom is first page after end of bitmap
    return (mem_bottom - 0x2000) + (page * 0x1000);
}

uint64_t pmm_free(uint64_t addr) {
    uint64_t page = (addr - (mem_bottom - 0x2000)) / 0x1000;
    pmm_set_free(page);
}
