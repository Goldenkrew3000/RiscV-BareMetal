#ifndef _PMM_H
#define _PMM_H

void pmm_init();
void pmm_set_used(uint64_t page);
void pmm_set_free(uint64_t page);
uint8_t pmm_is_page_used(uint64_t page);
uint64_t pmm_find_free_pages(uint64_t size);
uint64_t pmm_kalloc(int pages);
uint64_t pmm_free(uint64_t addr);

#endif
