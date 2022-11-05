include config.mk
.PHONY: all iso sys clean

QEMUOPTS = -kernel build/kernel.elf -m 128M -S  -nographic
all: sys
	cd kernel && make -s
qemu:
	qemu-system-i386 -kernel ./build/kernel.elf -nographic -gdb tcp::5050 -S
sys:
	-cd sys && make all
	-cd lib && make all
iso: build/kernel.elf
	-cp build/kernel.elf isodir/boot/kernel.bin
	nm build/kernel.elf | grep '[0-9|a-f]* [T|B] [^$$]*' > isodir/boot/kernel.map
	grub-mkrescue -o qnos.iso isodir
clean:
	cd kernel && make clean
	cd sys && make clean
	rm build/kernel.elf
count:
	find . -name "*[.h|.c|.sh]"|xargs cat|grep -v ^$|wc -l