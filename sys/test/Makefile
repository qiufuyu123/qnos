include ../../config.mk
SYS_NAME= test.sys
OBJS= main.o
#CC_FLAG=-m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fstrength-reduce -finline-functions 
all: $(OBJS)
	-rm ../../isodir/boot/$(SYS_NAME) 
	$(LD) $(LD_FLAG) -shared $(OBJS) -o $(SYS_NAME) 
	cp $(SYS_NAME) ../../isodir/boot/sys/
%.o:%.c
	$(CC) $(CC_FLAG) -I ../../include $< -o $@

%.o:%.asm
	$(NASM) $(NASM_FLAG) $< -o $@

clean:
	-rm $(OBJS) -rf
	-rm *.sys