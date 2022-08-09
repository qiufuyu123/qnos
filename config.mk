CC= gcc
LD=ld
NASM=nasm

CC_FLAG=-m32 -fno-builtin -fno-stack-protector -Xlinker -zmuldefs \
             -nostartfiles -lm -c
LD_FLAG= -m elf_i386
NASM_FLAG=-f elf