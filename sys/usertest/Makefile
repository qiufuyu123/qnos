include ../../config.mk
SYS_NAME= usertest.elf
OBJS= main.o 
CC_FLAG+= -Wl,--build-id=none
#CC_FLAG=-m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fstrength-reduce -finline-functions 
all: $(OBJS) 
	-rm ../../isodir/boot/$(SYS_NAME) 
	$(LD) $(LD_FLAG) $(OBJS) ../../lib/libc.a -Ttext=0x80000000 -e main -o $(SYS_NAME) 
	objcopy -O binary $(SYS_NAME) usertest.bin
	cp usertest.bin ../../isodir/boot/sys/
%.o:%.c
	$(CC) $(CC_FLAG) -I ../../lib/libc/include  $< -o $@

%.o:%.asm
	$(NASM) $(NASM_FLAG) $< -o $@

clean:
	rm $(OBJS) -rf
	rm *.sys