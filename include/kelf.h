#ifndef KERNEL_ELF_H
#define KERNEL_ELF_H

#include "types.h"
#pragma pack (1)
#define ELF_IDENTITY_MAGIC0     0x7F
#define ELF_IDENTITY_MAGIC1     'E'
#define ELF_IDENTITY_MAGIC2     'L'
#define ELF_IDENTITY_MAGIC3     'F'
#define ELF_IDENTITY_CLASS_NONE 0
#define ELF_IDENTITY_CLASS_32   1
#define ELF_IDENTITY_CLASS_64   2
#define ELF_IDENTITY_DATA_NONE  0
#define ELF_IDENTITY_DATA_2LSB  1
#define ELF_IDENTITY_DATA_2MSB  2

#define ELF_TYPE_NONE        0x0000
#define ELF_TYPE_RELOCATABLE 0x0001
#define ELF_TYPE_EXECUTABLE  0x0002
#define ELF_TYPE_DYNAMIC     0x0003
#define ELF_TYPE_CORE        0x0004
#define ELF_TYPE_LOPROC      0xFF00
#define ELF_TYPE_HIPROC      0xFFFF

#define ELF_MACHINE_NONE  0x0000
#define ELF_MACHINE_M32   0x0001
#define ELF_MACHINE_SPARC 0x0002
#define ELF_MACHINE_386   0x0003
#define ELF_MACHINE_68K   0x0004
#define ELF_MACHINE_88K   0x0005
#define ELF_MACHINE_860   0x0007
#define ELF_MACHINE_MIPS  0x0008

#define ELF_SECTION_INDEX_UNDEFINED 0x0000
#define ELF_SECTION_INDEX_LORESERVE 0xFF00
#define ELF_SECTION_INDEX_LOPROC    0xFF00
#define ELF_SECTION_INDEX_HIPROC    0xFF1F
#define ELF_SECTION_INDEX_ABS       0xFFF1
#define ELF_SECTION_INDEX_COMMON    0xFFF2
#define ELF_SECTION_INDEX_HIRESERVE 0xFFFF

#define ELF_SECTION_TYPE_NULL     0x00000000
#define ELF_SECTION_TYPE_PROGBITS 0x00000001
#define ELF_SECTION_TYPE_SYMTAB   0x00000002
#define ELF_SECTION_TYPE_STRTAB   0x00000003
#define ELF_SECTION_TYPE_RELA     0x00000004
#define ELF_SECTION_TYPE_HASH     0x00000005
#define ELF_SECTION_TYPE_DYNAMIC  0x00000006
#define ELF_SECTION_TYPE_NOTE     0x00000007
#define ELF_SECTION_TYPE_NOBITS   0x00000008
#define ELF_SECTION_TYPE_REL      0x00000009
#define ELF_SECTION_TYPE_SHLIB    0x0000000A
#define ELF_SECTION_TYPE_DYNSYM   0x0000000B
#define ELF_SECTION_TYPE_LOPROC   0x70000000
#define ELF_SECTION_TYPE_HIPROC   0x7FFFFFFF
#define ELF_SECTION_TYPE_LOUSER   0x80000000
#define ELF_SECTION_TYPE_HIUSER   0xFFFFFFFF

#define ELF_SECTION_FLAG_WRITE 0x00000001
#define ELF_SECTION_FLAG_ALLOC 0x00000002
#define ELF_SECTION_FLAG_EXEC  0x00000004
#define ELF_SECTION_FLAG_MASK  0xF0000000
typedef struct elf_header {
	char identify[16];
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint32_t entry;
	uint32_t phoffset;
	uint32_t shoffset;
	uint32_t flags;
	uint16_t header_size;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shcount;
	uint16_t shstrndx;
}elf_header_t;

#define ELF_HEADER_TYPE_NONE         0
#define ELF_HEADER_TYPE_OBJECT       1
#define ELF_HEADER_TYPE_EXECUTABLE   2
#define ELF_HEADER_TYPE_DYNAMIC      3
#define ELF_HEADER_TYPE_CORE         4

#define ELF_HEADER_MACHINE_I386   3
#define ELF_HEADER_MACHINE_ARM    40
#define ELF_HEADER_MACHINE_X86_64 62

#define ELF_HEADER_VERSION     1

typedef struct elf_program {
	uint32_t type;
	uint32_t offset;
	uint32_t vaddress;
	uint32_t paddr;
	uint32_t file_size;
	uint32_t memory_size;
	uint32_t flags;
	uint32_t align;
}elf_program_t;

#define ELF_PROGRAM_TYPE_LOADABLE 1

typedef struct elf_section {
	uint32_t name;
	uint32_t type;
	uint32_t flags;
	uint32_t address;
	uint32_t offset;
	uint32_t size;
	uint32_t link;
	uint32_t info;
	uint32_t alignment;
	uint32_t esize;
}elf_section_t;

#define ELF_SECTION_TYPE_NULL         0
#define ELF_SECTION_TYPE_PROGRAM      1
#define ELF_SECTION_TYPE_SYMBOL_TABLE 2
#define ELF_SECTION_TYPE_STRING_TABLE 3
#define ELF_SECTION_TYPE_RELA         4
#define ELF_SECTION_TYPE_HASH         5
#define ELF_SECTION_TYPE_DYNAMIC      6
#define ELF_SECTION_TYPE_NOTE         7
#define ELF_SECTION_TYPE_BSS          8

#define ELF_SECTION_FLAGS_WRITE    1
#define ELF_SECTION_FLAGS_MEMORY   2
#define ELF_SECTION_FLAGS_EXEC     8
#define ELF_SECTION_FLAGS_MERGE    16
#define ELF_SECTION_FLAGS_STRINGS  32
#define ELF_SECTION_FLAGS_INFO_LINK 64
#define ELF_SECTION_FLAGS_LINK_ORDER 128
#define ELF_SECTION_FLAGS_NONSTANDARD 256
#define ELF_SECTION_FLAGS_GROUP 512
#define ELF_SECTION_FLAGS_TLS 1024

//int a=sizeof(struct elf_section_header);
struct elf_symbol {
    uint32_t name;
    uint32_t value;
    uint32_t size;
    uint8_t info;
    uint8_t other;
    uint16_t shindex;
}PACKED;

struct elf_relocate {
    uint32_t offset;
    uint32_t info;
}PACKED;

struct elf_relocatea {
    uint32_t offset;
    uint32_t info;
    uint32_t addend;
}PACKED;

uint32_t elf_get_entry(void *address);
uint32_t elf_get_virtual(void *address);
uint32_t elf_get_symbol(void *address, const int8_t *name);
void elf_prepare(void *address);
void elf_relocate(void *address);

#endif