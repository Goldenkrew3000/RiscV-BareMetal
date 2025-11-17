CC="gcc"
CFLAGS="-I src -g -O0 -mcmodel=medany -ffreestanding -nostartfiles -nostdlib -nodefaultlibs"

rm -rf obj
mkdir obj

$CC $CFLAGS -c src/kmain.c -o obj/kmain.o
$CC $CFLAGS -c src/start.S -o obj/start.o
$CC $CFLAGS -c src/kprintf.c -o obj/kprintf.o
$CC $CFLAGS -c src/pmm.c -o obj/pmm.o
ld -T link.ld -o vmbsd.elf \
obj/start.o \
obj/kprintf.o \
obj/pmm.o \
obj/kmain.o

