include ../config.mk
OBJS= boot/loader.o init/main.o gates/gdt.o gates/idt.o gates/interrupt.o io.o string.o console.o printf.o hardware/timer.o hardware/ata.o\
	 mem/memorylayout.o mem/bitmap.o mem/page.o mem/pmm.o mem/vmm.o mem/malloc.o \
	 process/task.o process/process.o \
	 kobjects/kobject.o kobjects/objs/obj_kernel.o  kobjects/objs/obj_ata.o

all:$(OBJS)
	cd ../build &&  $(LD) $(LD_FLAG) -T ../kernel/linker.ld $(OBJS) -o kernel.elf
%.o:%.c
	$(CC) $(CC_FLAG) -I ../include $< -o ../build/$@

%.o:%.asm
	$(NASM) $(NASM_FLAG) $< -o ../build/$@

clean:
	cd ../build && rm $(OBJS) -rf