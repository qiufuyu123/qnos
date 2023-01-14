include ../../config.mk
SYS_NAME= libfont.a
OBJS= stdio.o string.o stdlib.o conio.o fcntl.o unistd.o
#CC_FLAG=-m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fstrength-reduce -finline-functions 
all: $(OBJS)
	-rm ../$(SYS_NAME) 
	$(LD) $(LD_FLAG) $(OBJS) -o $(SYS_NAME) 
	ar -rc ../$(SYS_NAME)  $(OBJS)
%.o:%.c
	$(CC) $(CC_FLAG) -c -I ./include $< -o $@
%.o:%.asm
	$(NASM) $(NASM_FLAG) $< -o $@

clean:
	-rm $(OBJS) -rf
	-rm $(SYS_NAME)
	-rm ../$(SYS_NAME)