#! /bin/bash
src='
include ../../config.mk
SYS_NAME=U
OBJS= main.o
CC_FLAG+= -Wl,--build-id=none
all: $(OBJS)
	-rm ../../isodir/boot/sys/$(SYS_NAME).elf
	$(LD) $(LD_FLAG) $(OBJS) ../../lib/libc.a -Ttext=0x80000000 -e main -o $(SYS_NAME).elf
	cp $(SYS_NAME).elf ../../isodir/boot/sys/
%.o:%.c
	$(CC) $(CC_FLAG) -I ../../lib/libc/include  $< -o $@
%.o:%.asm
	$(NASM) $(NASM_FLAG) $< -o $@
clean:
	-rm $(OBJS) -rf
	-rm *.elf
'
echo $#
if [ $# -eq 1 ]; then
    echo 'Going to create a user program:'$1
    echo 'Making Dir...'
    mkdir ./sys/$1
    echo  "${src/U/$1}"> ./sys/$1/Makefile
    echo 'Writing Makefile...'
    echo "//Start Your Codes here:" > ./sys/$1/main.c
    echo 'Writing source file main.c ...'
    echo '[OK]: Modify ./sys/Makefile to apply'
else
    echo 'Bad Args!'
fi