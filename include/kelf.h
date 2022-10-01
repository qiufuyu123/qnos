#pragma once

/*
elf文件加载（动态加载运行so）
可用于嵌入式程序
可用于Windows程序加载so

用法流程：
1 读取elf文件内容
2 计算内存大小 elf_memory_size
3 分配该大小可执行内存
4 调用模块初始化 elf_module_init
5 调用入口函数或指定名称的函数



对于加载的elf目前存在限制：
不支持依赖
不支持非static全局变量
不支持重定向表

*/

#include "types.h"
//#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PACKED
#ifdef _MSC_VER
#define PACKED
#define USE_PRAGMA_PACK
#pragma warning(disable : 4819)
#else
#define PACKED __attribute__((packed))
#endif
#endif

#ifdef USE_PRAGMA_PACK
#pragma pack(push, 1)
#endif


// elf模块
typedef struct elf_module elf_module;

//验证elf格式是否正确，是否可加载， 正常返回0， 异常返回非0
int elf_check(const uint8_t *elfdata, size_t elflen);

//计算需分配内存大小
size_t elf_memory_size(const uint8_t *elfdata, size_t elflen);

//初始化elf模块
// mem 提前分配好的内存，需要可执行权限（需要内存大小可通过elf_memory_size计算得到）
elf_module *elf_module_init(uint8_t *elfdata);

//获取入口函数地址
void *elf_module_entry(elf_module *m);

//获取函数地址
void *elf_module_sym(elf_module *m, const char *name);
void elf_module_release(elf_module *m);
// elf_check 返回结果
#define ELF_CHECK_OK 0
#define ELF_CHECK_ERR_LENGTH -1
#define ELF_CHECK_ERR_MAGIC -2
#define ELF_CHECK_ERR_TYPE -3
#define ELF_CHECK_ERR_CLASS -4
#define ELF_CHECK_ERR_ENDIAN -5
#define ELF_CHECK_ERR_MACHINE -6



// elf 头

typedef struct PACKED elf32_header {
    uint8_t magic[4];  // 固定头
    uint8_t cls;  //32位/64位
    uint8_t endian;  //大小端
    uint8_t rev;  
    uint8_t abi;  //abi类型
    uint8_t pad[8]; 
    uint16_t type;      // 文件类型
    uint16_t machine;   // cpu类型
    uint32_t version;   // 文件版本
    uint32_t entry;    // 入口函数偏移
    uint32_t phoff;    // program header 偏移
    uint32_t shoff;    // section header 偏移
    uint32_t flags;     // 选项
    uint16_t ehsize;    // elf header 大小
    uint16_t phentsize; // program header 大小
    uint16_t phnum;     // program header 个数
    uint16_t shentsize; // section header  大小
    uint16_t shnum;     // section header  个数
    uint16_t shstrndx;  // section header  字符串表索引
} elf32_header;

typedef struct PACKED elf64_header {
    uint8_t magic[4];  // 固定头
    uint8_t cls;  //32位/64位
    uint8_t endian;  //大小端
    uint8_t rev;  
    uint8_t abi;  //abi类型
    uint8_t pad[8];
    uint16_t type;      // 文件类型
    uint16_t machine;   // cpu类型
    uint32_t version;   // 文件版本
    uint64_t entry;    // 入口函数偏移
    uint64_t phoff;    // program header 偏移
    uint64_t shoff;    // section header 偏移
    uint32_t flags;     // 选项
    uint16_t ehsize;    // elf header 大小
    uint16_t phentsize; // program header 大小
    uint16_t phnum;     // program header 个数
    uint16_t shentsize; // section header  大小
    uint16_t shnum;     // section header  个数
    uint16_t shstrndx;  // section header  字符串表索引
} elf64_header;

typedef struct PACKED elf32_program_header {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
} elf32_program_header;

typedef struct PACKED elf64_program_header {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
} elf64_program_header;

typedef struct {
    int32_t    d_tag;    /* controls meaning of d_val */
    union {
        uint32_t d_val;    /* Multiple meanings - see d_tag */
        uint32_t d_ptr;    /* program virtual address */
    } d_un;
} elf_dynentry;
typedef struct PACKED elf32_section_header {
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t addr;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t align;
    uint32_t entsize;
} elf32_section_header;

typedef struct PACKED elf64_section_header {
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t align;
    uint64_t entsize;
} elf64_section_header;


typedef struct PACKED elf32_symbol {
    uint32_t name;          //字符串表索引
    uint32_t value;        //值
    uint32_t size;          //大小
    uint8_t type : 4;       //类型
    uint8_t bind : 4;       // bind类型
    uint8_t visibility : 2; //可见性
    uint8_t rev : 6;
    uint16_t shndx; 
} elf32_symbol;

typedef struct PACKED elf64_symbol {
    uint32_t name;          //字符串表索引
    uint8_t type : 4;       //类型
    uint8_t bind : 4;       // bind类型
    uint8_t visibility : 2; //可见性
    uint8_t rev : 6;
    uint16_t shndx;  // section索引
    uint64_t value; //值
    uint64_t size;  //大小
} elf64_symbol;


#define ELF_MAGIC_0 0x7f
#define ELF_MAGIC_1 'E'
#define ELF_MAGIC_2 'L'
#define ELF_MAGIC_3 'F'


#define ELF_TYPE_REL 1  //编译中间 .o
#define ELF_TYPE_EXEC 2 // 可执行文件 
#define ELF_TYPE_DYN 3  // 动态库 .so

#define ELF_MACHINE_X86 3     
#define ELF_MACHINE_X86_64 62 
#define ELF_MACHINE_ARM 40  
#define ELF_MACHINE_AARCH64 183

#define ELF_CLASS_32 1
#define ELF_CLASS_64 2

#define ELF_ENDIAN_LITTLE 1 //小端
#define ELF_ENDIAN_BIG   2 //大端





#define ELF_PROGRAM_TYPE_LOAD 1
#define ELF_PROGRAM_TYPE_DYNAMIC 2
#define ELF_PROGRAM_TYPE_INTERP 3
#define ELF_PROGRAM_TYPE_NOTE 4
#define ELF_PROGRAM_TYPE_SHLIB 5
#define ELF_PROGRAM_TYPE_PHDR 6
#define ELF_PROGRAM_TYPE_TLS 7

#define ELF_PROGRAM_FLAG_X (1 << 0)
#define ELF_PROGRAM_FLAG_W (1 << 1)
#define ELF_PROGRAM_FLAG_R (1 << 2)

#define ELF_SECTION_TYPE_PROGBITS 1
#define ELF_SECTION_TYPE_SYMTAB 2
#define ELF_SECTION_TYPE_STRTAB 3
#define ELF_SECTION_TYPE_RELA 4
#define ELF_SECTION_TYPE_HASH 5
#define ELF_SECTION_TYPE_DYNAMIC 6
#define ELF_SECTION_TYPE_NOTE 7
#define ELF_SECTION_TYPE_NOBITS 8
#define ELF_SECTION_TYPE_REL 9
#define ELF_SECTION_TYPE_SHLIB 10
#define ELF_SECTION_TYPE_DYNSYM 11
#define ELF_SECTION_TYPE_INIT_ARRAY 14
#define ELF_SECTION_TYPE_FINI_ARRAY 15
#define ELF_SECTION_TYPE_PREINIT_ARRAY 16
#define ELF_SECTION_TYPE_GROUP 17
#define ELF_SECTION_TYPE_SYMTAB_SHNDX 18

#define ELF_SECTION_FLAG_WRITE (1 << 0)
#define ELF_SECTION_FLAG_ALLOC (1 << 1)
#define ELF_SECTION_FLAG_EXECINSTR (1 << 2)
#define ELF_SECTION_FLAG_MERGE (1 << 4)
#define ELF_SECTION_FLAG_STRINGS (1 << 5)
#define ELF_SECTION_FLAG_INFO_LINK (1 << 6)
#define ELF_SECTION_FLAG_LINK_ORDER (1 << 7)
#define ELF_SECTION_FLAG_OS_NONCONFORMING (1 << 8)
#define ELF_SECTION_FLAG_GROUP (1 << 9)
#define ELF_SECTION_FLAG_TLS (1 << 10)
#define ELF_SECTION_FLAG_COMPRESSED (1 << 11)


#define ELF_SYMBOL_BIND_LOCAL 0
#define ELF_SYMBOL_BIND_GLOBAL 1
#define ELF_SYMBOL_BIND_WEAK 2

#define ELF_SYMBOL_TYPE_NOTYPE 0
#define ELF_SYMBOL_TYPE_OBJECT 1
#define ELF_SYMBOL_TYPE_FUNC 2
#define ELF_SYMBOL_TYPE_SECTION 3
#define ELF_SYMBOL_TYPE_FILE 4
#define ELF_SYMBOL_TYPE_COMMON 5
#define ELF_SYMBOL_TYPE_TLS 6

#define ELF_SYMBOL_VISIBILITY_DEFAULT 0
#define ELF_SYMBOL_VISIBILITY_INTERNAL 1
#define ELF_SYMBOL_VISIBILITY_HIDDEN 2
#define ELF_SYMBOL_VISIBILITY_PROTECTED 3



#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif