OUTPUT_ARCH("riscv")
ENTRY(_start)

MEMORY
{
  RAM (rwx) : ORIGIN = 0x80200000, LENGTH = 128M
}

SECTIONS
{
  /*. = 0x80200000;*/

  .text : ALIGN(4K) {
    *(.init);
    *(.text);
  }
  
  .bss : ALIGN(4K) {
    PROVIDE(bss_start = .);
    *(.bss)
    . += 4096;
    PROVIDE(stack_top = .);
    . += 4096;
    PROVIDE(global_ptr = .);
    PROVIDE(bss_end = .);
  }

  .rodata : ALIGN(4K) {
    *(.rodata);
  }

  .data : ALIGN(4K) {
    *(.data);
  }

  PROVIDE(kern_end = .);
}
