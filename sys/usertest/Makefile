
include ../../config.mk
SYS_NAME=usertest
OBJS= main.o
CC_FLAG+= -Wl,--build-id=none
all: $(OBJS)
	-rm ../../isodir/boot/sys/$(SYS_NAME)
	$(LD) $(LD_FLAG) $(OBJS) ../../lib/libc.a -Ttext=0x80000000 -e main -o $(SYS_NAME)
	cp $(SYS_NAME) ../../isodir/boot/sys/
%.o:%.c
	$(CC) $(CC_FLAG) -I ../../lib/libc/include  $< -o $@
%.o:%.asm
	$(NASM) $(NASM_FLAG) $< -o $@
clean:
	-rm $(OBJS) -rf
	-rm $(SYS_NAME)

