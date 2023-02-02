include ../../config.mk
SYS_NAME= libc.a
OBJS= stdio.o string.o stdlib.o conio.o fcntl.o unistd.o qnos.o malloc.o
#CC_FLAG=-m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fstrength-reduce -finline-functions 
all: $(OBJS)
	-rm ../$(SYS_NAME) 
	$(LD) $(LD_FLAG) $(OBJS) -o $(SYS_NAME) 
	ar -rc ../$(SYS_NAME)  $(OBJS)
	#cp $(SYS_NAME) ../../isodir/boot/sys/
%.o:%.c
	$(CC) $(CC_FLAG) -c -I ./include $< -o $@

%.o:%.asm
	$(NASM) $(NASM_FLAG) $< -o $@

clean:
	-rm $(OBJS) -rf
	-rm libc.a
	-rm ../libc.a