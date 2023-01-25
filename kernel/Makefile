include ../config.mk
OBJS= boot/loader.o init/main.o gates/gdt.o gates/idt.o gates/interrupt.o io.o string.o console.o printf.o list.o sysmodule.o \
	hardware/devices.o hardware/timer.o hardware/ata.o hardware/vga.o hardware/keyboard/keyboard.o hardware/ramdisk.o hardware/framebuffer.o hardware/kbd.o hardware/mouse.o\
	 mem/memorylayout.o mem/bitmap.o mem/page.o mem/pmm.o mem/vmm.o mem/malloc.o \
	 process/task.o process/process.o process/sync.o process/symbol.o elf.o process/user_task.o process/syscall.o \
	 process/ipc/pipe.o  \
	 kobjects/kobject.o kobjects/objs/obj_kernel.o kobjects/objs/obj_ktty.o kobjects/objs/obj_serial.o kobjects/objs/obj_stdio.o kobjects/objs/obj_fs.o \
	 utils/fastmapper.o utils/qhash.o utils/qconfig.o utils/hashmap.o utils/circlequeue.o \
	 fs/fs_iso9660.o fs/fs_fat32.o

all: $(OBJS)
	cd ../build &&  $(LD) $(LD_FLAG) -T ../kernel/linker.ld $(OBJS) -o kernel.elf
%.o:%.c
	@ $(CC) $(CC_FLAG) -I ../include $< -o ../build/$@

%.o:%.asm
	@ $(NASM) $(NASM_FLAG) $< -o ../build/$@

clean:
	cd ../build && rm $(OBJS) -rf