include config.mk

all:
	cd kernel && make
iso: build/kernel.elf
	-cp build/kernel.elf isodir/boot/kernel.bin
	grub-mkrescue -o qnos.iso isodir
clean:
	cd kernel && make clean
	rm build/kernel.elf