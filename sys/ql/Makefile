include ../../config.mk
ELF2OBJ=../../tools/toolchain/qnelf2bin
SYS_NAME= ql.elf
OBJS= main.o ql.o
CC_FLAG+= -Wl,--build-id=none
#CC_FLAG=-m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fstrength-reduce -finline-functions 
all: $(OBJS) 
	-rm ../../isodir/boot/sys/ql.elf 
	$(LD) $(LD_FLAG) $(OBJS) ../../lib/libc.a -Ttext=0x80000000 -e main -o $(SYS_NAME) 
	cp ql.elf ../../isodir/boot/sys/
%.o:%.c
	$(CC) $(CC_FLAG) -I ../../lib/libc/include  $< -o $@

%.o:%.asm
	$(NASM) $(NASM_FLAG) $< -o $@

clean:
	-rm $(OBJS) -rf
	-rm *.elf