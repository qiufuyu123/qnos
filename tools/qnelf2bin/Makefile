include ../../config.mk
TOOL_NAME= qnelf2bin
SRC= main.c
#CC_FLAG=-m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fstrength-reduce -finline-functions 
all: $(OBJS)
	#rm ../../isodir/boot/$(SYS_NAME) 
	#$(LD)  $(OBJS) -o $(TOOL_NAME) 

	
	$(CC) $(SRC) -o $(TOOL_NAME)
	cp $(TOOL_NAME) ../toolchain
clean:
	-rm $(TOOL_NAME)
	-rm ../toolchain/$(TOOL_NAME)