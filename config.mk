CC= gcc
LD=ld
NASM=nasm
#-nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fstrength-reduce -finline-functions 
CC_FLAG=-m32 -Wall -c -ffreestanding -fno-pie -g -std=gnu99
LD_FLAG= -m elf_i386
NASM_FLAG=-f elf