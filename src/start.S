/*
 * The hartid is passed through a0
 * The device tree is passed through a1
 */

.section .init

.global _start
_start:
    /* Set global pointer */
.option push
.option norelax
    la gp, global_ptr
.option pop

    /* Set the trap handler - Must happen after initializing GP */
    la t0, trap_entry
    csrw stvec, t0 /* Supervisor trap vector base address register (CSR Write) */

    /* Reset SATP */
    csrw satp, zero

    /* Clear BSS */
    la t5, bss_start
    la t6, bss_end
bss_clear:
    sd zero, (t5)
    addi t5, t5, 8
    bltu t5, t6, bss_clear

    /* Setup stack */
    la sp, stack_top
    add s0, sp, zero

    /* Jump to kmain */
    jal zero, kmain

.section .text

#define REGBYTES 8

.align 6
.global trap_entry
trap_entry:
    addi sp, sp, -32 * REGBYTES
    sd x0, (0 * REGBYTES)(x5)
    jal handle_trap
