include config.mk
.PHONY: all iso clean
all:
	cd kernel && make
	cd sys && make all
iso: build/kernel.elf
	-cp build/kernel.elf isodir/boot/kernel.bin
	nm build/kernel.elf | grep '[0-9|a-f]* T [^$$]*' > isodir/boot/kernel.map
	grub-mkrescue -o qnos.iso isodir
clean:
	cd kernel && make clean
	cd sys && make clean
	rm build/kernel.elf