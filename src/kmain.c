/*
 * Info
 * UART0 - 0x04140000, Initialized to 115200. I think it's 8250 compatible?
 * Load address: 0x80200000

   Memory Layout
   0x80000000 - 0x80200000 (0x200000) -> Manually reserved for fw monitor
   0x80200000 - kern_end (Page aligned) -> Kernel space
   kern_end - 0x8f000000 -> Free memory that shouldn't interact with anything reserved

 */

#define MEM_TOP 0x8f000000

#include <stdint.h>
#include <stddef.h>
#include <kprintf.h>
#include <pmm.h>

#define UART0_BASE 0x04140000

extern uint64_t stack_top;
extern uint64_t global_ptr;
extern uint64_t kern_end;	// Last bit of kernel room, probably not page aligned

static inline void mmio_write_u8(unsigned long reg, unsigned int offset, uint8_t val) {
    (*(volatile uint32_t*)((reg) + (offset))) = val;
}

size_t kern_strlen(const char *str)
{
    const char *s;

    s = str;
    while (1) {
        if (*s == '\0')
            break;
        s++;
    }

    return (s - str);
}

void sbi_srst() {
    register int a7 asm("a7") = 0x53525354;	// (EID) SRST
    register int a6 asm("a6") = 0x1;		// (FID) Cold Reboot
    asm volatile("ecall" : : "r"(a6), "r"(a7) : "memory");
}

void kmain(void) {
    printf("\nBSD/Hojuix Developer Pre-Alpha (riscv64) - Built %s %s\n", __DATE__, __TIME__);
    printf("Using OpenSBI UART output\n\n");

    // Print stack pointer and global pointer
    uint64_t sp, gp, fdt, kern_end_addr = 0;
    asm volatile("mv %0, sp" : "=r"(sp));
    asm volatile("mv %0, gp" : "=r"(gp));
    //asm volatile("mv %0, a1" : "=r"(fdt));
    // ISSUE: Register a1 does not seem to be held correctly to here... Hard coding FDT addr for now
    fdt = 0x8f281730;
    printf("SP is at 0x%llx\n", sp);
    printf("GP is at 0x%llx\n", gp);
    printf("DT is at 0x%llx\n", fdt);

    // FDT Check
    printf("First 16 bytes of FDT:\n");
    uint8_t* fdt_ptr = (uint8_t*)fdt;
    for (int i = 0; i < 16; i++) {
        printf("0x%.2x ", fdt_ptr[i]);
    } printf("\n\n");

    // Initialize memory structures
    pmm_init();

    // Test PMM
    printf("PMM Test\n");
    uint64_t p1 = pmm_kalloc(1);
    printf("Page 1: 0x%llx\n", p1);
    uint64_t p2 = pmm_kalloc(1);
    printf("Page 2: 0x%llx\n", p2);
    uint64_t p3 = pmm_kalloc(2);
    printf("Page 3: 0x%llx\n", p3);
    uint64_t p4 = pmm_kalloc(1);
    printf("Page 4: 0x%llx\n", p4);

    // Test free p3 (one page at a time)
    pmm_free(p3);
    pmm_free(p3 + 0x1000);

    uint64_t p5 = pmm_kalloc(3);
    printf("Page 5: 0x%llx\n", p5);
    uint64_t p6 = pmm_kalloc(1);
    printf("Page 6: 0x%llx\n", p6);


    uint64_t value = 0;
    asm volatile("csrr a0, sstatus" : "=r"(value) : : "a0");
    printf("status: 0x%llx\n", value);

    // Perform OpenSBI SRST (https://www.scs.stanford.edu/~zyedidia/docs/riscv/riscv-sbi.pdf - P32)
    sbi_srst();

    while(1) { asm volatile("nop"); }
}

void handle_trap() {
    printf("Exception/Interrupt trapped.\n");
}
