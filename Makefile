include config.mk
.PHONY: all iso sys clean

QEMUOPTS = -kernel build/kernel.elf -m 128M -S  -nographic
tool:
	cd tools && make all
all: sys
	cd kernel && make -s
qemu:
	qemu-system-i386 -kernel ./build/kernel.elf -nographic -gdb tcp::6666 -S
sys:
	-cd sys && make all
	-cd lib && make all
iso: build/kernel.elf
	-cp build/kernel.elf isodir/boot/kernel.bin
	nm build/kernel.elf | grep '[0-9|a-f]* [T|B] [^$$]*' > isodir/boot/kernel.map
	grub-mkrescue -o qnos.iso isodir
	qemu-img convert -f raw -O vmdk a.img disk.vmdk
clean:
	cd kernel && make clean
	cd sys && make clean
	cd tools && make clean
	cd lib && make clean
	-rm isodir/boot/sys/*.*
	-rm isodir/boot/kernel.bin
	-rm build/kernel.elf
	-rm qnos.iso
	-rm *.img
	-rm *.vmdk
count:
	find . -name "*[.h|.c|.sh]"|xargs cat|grep -v ^$|wc -l