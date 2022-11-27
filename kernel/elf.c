// #include"kelf.h"
// #include"string.h"
// #include"process/symbol.h"
// #include"console.h"
// size_t elf_memory_size(const uint8_t * elfdata, size_t elflen){
//     elf_header_t* h = (elf_header_t*)elfdata;
//     elf_program_t* ph;
//     size_t i;
//     size_t vstart = (size_t)-1; //最小虚拟内存地址 
//     size_t vend = 0;  //最大虚拟内存地址 
//     size_t curvend;
//     ph = (elf_program_t*)(elfdata + h->phoffset);
//     for(i = 0; i < h->phnum; i++){
//         if(ph->type == ELF_PROGRAM_TYPE_LOAD){
//             if(ph->vaddress < vstart) vstart = ph->vaddress /* & ALIGN_VADDR*/;
//             curvend = ph->vaddress + ph->memory_size;
//             if(curvend > vend) vend = curvend;
//         }
//         ph ++; 
//     }
//     printf("calced vstart: %d vend: %d\n", (void*)vstart,  (void*)vend);
//     if(vstart > vend)return 0;    
//     return vend - vstart + sizeof(elf_module);
// }


// uint32_t elf_get_entry(void *address) {
//     struct elf_header *header = get_header(address);

//     if (!header)
//         return 0;

//     return (uint32_t) header->entry;
// }

// uint32_t elf_get_virtual(void *address) {
//     struct elf_program *pheader;
//     struct elf_header *header = get_header(address);

//     if (!header)
//         return 0;

//     pheader = (struct elf_program *)((uint32_t) address + header->phoffset);

//     return (uint32_t) pheader->vaddress;
// }

// uint32_t elf_get_symbol(void *address, const int8_t *name) {
//     struct elf_section *sheader, *relHeader, *symHeader;
//     struct elf_symbol *symTable, *symEntry;
//     int8_t *infoTable, *strTable;
//     uint32_t i, count;
//     struct elf_header *header = get_header(address);

//     if (!header)
//         return 0;

//     sheader = (struct elf_section_header *)((uint32_t) address + header->shoffset);
//     relHeader = &sheader[2];
//     symHeader = &sheader[relHeader->link];

//     symTable = (struct elf_symbol *)((uint32_t) address + symHeader->offset);
//     infoTable = (int8_t *)address + sheader[relHeader->info].offset;
//     strTable =  (int8_t *)address + sheader[symHeader->link].offset;

//     count = symHeader->size / symHeader->esize;

//     for (i = 0; i < count; i++) {
//         symEntry = &symTable[i];
//         //("sym:%s;",strTable+symEntry->name);
//         if (!memcmp(name, strTable + symEntry->name, strlen(name)))
//             return (uint32_t)(infoTable + symEntry->value);
//     }

//     return 0;
// }

// void elf_prepare(void *address) {
//     uint32_t i;
//     struct elf_section*sheader;
//     struct elf_header *header = get_header(address);

//     if (!header)
//         return;

//     sheader = (struct elf_section *)((uint32_t) address + header->shoffset);

//     for (i = 0; i < header->shcount; i++)
//         if (sheader[i].type == 8)
//             memclr((int8_t *)address + sheader[i].offset, sheader[i].size);
// }

// int elf_relocate(void *address) {
//     struct elf_section *sheader, *relHeader, *symHeader;
//     struct elf_relocate *relTable, *relEntry;
//     struct elf_symbol *symTable, *symEntry;
//     int8_t *infoTable, *strTable;
//     uint32_t i, count, *entry, value, addend;
//     uint8_t type, index;
//     struct elf_header *header = get_header(address);

//     if (!header){
//         //("bad elf header!;");
//         return -1;
//     }
//     //("a");
//     sheader = (struct elf_section *)((uint32_t) address + header->shoffset);
//     relHeader = &sheader[5];
//     symHeader = &sheader[relHeader->link];
//     //("a2");
//     relTable = (struct elf_relocate *)((uint32_t) address + relHeader->offset);
//     symTable = (struct elf_symbol *)((uint32_t) address + symHeader->offset);
//     infoTable = (int8_t *)address + sheader[relHeader->info].offset;
//     strTable =  (int8_t *)address + sheader[symHeader->link].offset;
//     //("a3 %d / %d",relHeader->size,relHeader->esize);
//     count = relHeader->size / relHeader->esize;
//     //("a4");
//     ////("b%d;",count);
//     //("a5");
//     for (i = 0; i < count; i++) {
//         relEntry = &relTable[i];

//         type =  relEntry->info & 0x0F;
//         index = relEntry->info >> 8;

//         symEntry = &symTable[index];
//         entry = (uint32_t *)(infoTable + relEntry->offset);
//         value = *entry;
//         printf("try to relocate:%s;",strTable + symEntry->name);
//         addend = (symEntry->shindex) ? (uint32_t) address + sheader[symEntry->shindex].offset + symEntry->value : symbol_find(strTable + symEntry->name);

//         switch (type) {
//             case 1:
//                 *entry = value + addend;
//                 break;
//             case 2:
//                 *entry = value + addend - (uint32_t) entry;
//                 break;
//             default:
//                 printf("unknow:%d;%s",strTable + symEntry->name);
//                 break;
//         }
//     }
//     //elf_relocate_rodata(address,5);
//     //elf_relocate_rodata(address,7);
//     return 0;
// }
#include"mem/memorylayout.h"
#include "kelf.h"
#include "console.h"
#include "string.h"
#include"process/symbol.h"
#include"mem/malloc.h"
#if 0
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

//#if __SIZEOF_POINTER__ == 4  //32位

#define ELF_HOST_CLASS ELF_CLASS_32
typedef elf32_header elf_header;
typedef elf32_program_header elf_program_header;
typedef elf32_section_header elf_section_header;
typedef elf32_symbol elf_symbol;

// #else //64位
// #define ELF_HOST_CLASS ELF_CLASS_64
// typedef elf64_header elf_header;
// typedef elf64_program_header elf_program_header;
// typedef elf64_section_header elf_section_header;
// typedef elf64_symbol elf_symbol;
// #endif

#if defined(__x86_64__) || defined(_WIN64)
#define ELF_HOST_MACHINE ELF_MACHINE_X86_64
#elif defined(__i686__) || defined(__i586__) || defined(_WIN32)
#define ELF_HOST_MACHINE ELF_MACHINE_X86
#elif defined(__arm__)
#define ELF_HOST_MACHINE ELF_MACHINE_ARM
#elif defined(__aarch64__)
#define ELF_HOST_MACHINE ELF_MACHINE_AARCH64
#elif defined(__mips__)
#define ELF_HOST_MACHINE 8
#else
#define ELF_HOST_MACHINE 0 
#endif

struct elf_module{
    size_t vstart; //最小虚拟地址
    uint8_t* elfdata;
    void* entry;
    elf_symbol* sym;
    size_t symnum;
    char* strtab;
    size_t strtabsz;
    uint32_t use_pgn;
};
struct elf_relocate
{
    uint32_t offset;
    uint32_t info;
};
char *elf_solve_str(elf_header *h,uint32_t idx)
{
    elf_section_header*sheader = (elf_section_header *)((uint32_t) h + h->shoff);  
    return (uint32_t)h+sheader[h->shstrndx].offset +idx;
}
int elf_relocate(elf_section_header *rel,char *address,uint32_t new_base)
{
    elf_section_header *sheader, *relHeader, *symHeader;
    struct elf_relocate *relTable, *relEntry;
    elf_symbol *symTable, *symEntry;
    int8_t *infoTable, *strTable;
    uint32_t i, count, *entry, value, addend;
    uint8_t type, index;
    elf_header *header = address;

    if (!header){
        //("bad elf header!;");
        return -1;
    }
    //("a");
    sheader = (elf_section_header *)((uint32_t) address + header->shoff);
    relHeader = rel;
    symHeader = &sheader[relHeader->link];
    //("a2");
    relTable = (struct elf_relocate *)((uint32_t) address + relHeader->offset);
    symTable = (struct elf_symbol *)((uint32_t) address + symHeader->offset);
    infoTable = (int8_t *)address + sheader[relHeader->info].offset;
    strTable =  (int8_t *)address + sheader[symHeader->link].offset;
    //("a3 %d / %d",relHeader->size,relHeader->esize);
    count = relHeader->size / relHeader->entsize;
    //("a4");
    //printf("b%d;",count);
    //("a5");
    for (i = 0; i < count; i++) {
        relEntry = &relTable[i];

        type =  relEntry->info & 0x0F;
        index = relEntry->info >> 8;

        symEntry = &symTable[index];
        //entry = (uint32_t *)(infoTable + relEntry->offset);
        entry=new_base+relEntry->offset;
        value = *entry;
        //printf("try to relocate:%s (%d,%x);",strTable + symEntry->name,value,new_base);
        addend = (symEntry->shndx) ? (uint32_t) new_base + sheader[symEntry->shndx].offset + symEntry->value : symbol_find(strTable + symEntry->name);
        //uint32_t S=new_base + sheader[symEntry->shndx].offset + symEntry->value;
        if(symEntry->shndx&&(type==1))addend=new_base+symEntry->value;
        //printf("S:0x%x e:%x;",addend,entry);
        uint32_t S=entry;
        switch (type) {
            case 1:
                *entry = value + addend;
                printf("old%x->new%x;",value,*entry);
                break;
            case 2:
                //printf("S:%x A:%d P:%x;",S,value,addend);
                //S+A-P
                //Note : really interesting exper (i dont really understand why)
                *entry = value+addend-S;
                break;
            case 8:
                printf("old:%x",value);
                *entry=value+new_base;
                printf("new:%x;",*entry);
                break;
            default:
                printf("unknow:%d;%s",strTable + symEntry->name);
                break;
        }
        //printf("after rel:%x;",*entry);
    }
    return 0;
}
uint32_t qbinary_load(char *bindata,uint32_t dest_,uint32_t size)
{
    QNBinary_t *bhead=bindata;
    if(bhead->magic[0]=='Q'&&bhead->magic[1]=='B'&&bhead->magic[2]=='F')
    {
        printf("checked magic!");
        uint32_t r= bhead->v_entry;
        memcpy(dest_,bindata+sizeof(QNBinary_t),size);
        return r;
    }
    printf(bhead->magic);
    return 0;
}
//验证elf格式是否正确，是否可加载， 正常返回0， 异常返回非0
int elf_check(const uint8_t * d, size_t elflen){
    elf_header* h = (elf_header*)d;
    uint32_t endian_test = 0x02000001;
    uint8_t* plendian = (uint8_t*)&endian_test;
    if(elflen <= sizeof(elf_header))return ELF_CHECK_ERR_LENGTH;
    if(h->magic[0] != ELF_MAGIC_0 
        || h->magic[1] != ELF_MAGIC_1 
        || h->magic[2] != ELF_MAGIC_2 
        || h->magic[3] != ELF_MAGIC_3 
    )return ELF_CHECK_ERR_MAGIC;
    if(h->cls != ELF_HOST_CLASS)return ELF_CHECK_ERR_CLASS;
    if(h->endian != *plendian)return ELF_CHECK_ERR_ENDIAN;
    
    if(h->type != ELF_TYPE_DYN && h->type != ELF_TYPE_EXEC)return ELF_CHECK_ERR_TYPE;
    if(ELF_HOST_MACHINE && h->machine != ELF_HOST_MACHINE)return ELF_CHECK_ERR_MACHINE;
    return 0;
}


//计算需额外分配内存大小
size_t elf_memory_size(const uint8_t * elfdata, size_t elflen){
    elf_header* h = (elf_header*)elfdata;
    elf_program_header* ph;
    size_t i;
    size_t vstart = (size_t)-1; //最小虚拟内存地址 
    size_t vend = 0;  //最大虚拟内存地址 
    size_t curvend;
    ph = (elf_program_header*)(elfdata + h->phoff);
    for(i = 0; i < h->phnum; i++){
        if(ph->type == ELF_PROGRAM_TYPE_LOAD){
            if(ph->vaddr < vstart) vstart = ph->vaddr /* & ALIGN_VADDR*/;
            curvend = ph->vaddr + ph->memsz;
            if(curvend > vend) vend = curvend;
        }
        ph ++; 
    }
    DEBUG_PRINTF("calced vstart: %d vend: %d\n", (void*)vstart,  (void*)vend);
    if(vstart > vend)return 0;    
    return vend - vstart + sizeof(elf_module);
}


//初始化elf模块
elf_module* elf_module_init( uint8_t * elfdata){
    elf_header* h = (elf_header*)elfdata;
    elf_program_header* ph;
    elf_program_header* psh = NULL;
    elf_section_header* sh;
    elf_section_header* basesh;
    size_t i;     
    elf_module* elf = kmalloc(sizeof(elf_module));
    //printf("alloc elf:%x",elf);
    if(!elf)return 0;
    //uint8_t* ptr ;//= mem + sizeof(elf_module);
    uint8_t* newbase;
    size_t vstart = (size_t)-1;
    size_t vend = 0;
    size_t curvend;
    size_t sz;
    uint8_t* psrc;
	elf_symbol* sym = NULL;
	
    //计算虚拟地址
    vstart = (size_t)0;
    ph = (elf_program_header*)(elfdata + h->phoff);
    for(i = 0; i < h->phnum; i++){
        if(ph->type == ELF_PROGRAM_TYPE_LOAD){
            if(!psh)psh = ph;
            if(ngx_align(ph->vaddr+ph->memsz,ph->align)>vend)vend=  ngx_align(ph->vaddr+ph->memsz,ph->align);     
        }
        ph ++; 
    }
    basesh = (elf_section_header*)(elfdata + h->shoff);
    sh = basesh;
    for(i = 0; i < h->shnum; i++){
        if(!strcmp(elf_solve_str(h,sh->name),".data")||!strcmp(elf_solve_str(h,sh->name),".rpdata")||!strcmp(elf_solve_str(h,sh->name),".bss"))
        {
            //printf("find %s section :0x%x sz:%d",elf_solve_str(h,sh->name),sh->addr,sh->size);
            if(sh->addr+sh->size>vend)vend=sh->addr+sh->size;
        }
        sh++;
    }
    if(vstart > vend)
    {
        //printf("VSTART:%x",vstart);
        kfree(elf);
        return 0;
    }
    uint32_t need_sz=vend-vstart;

    //if(vend - vstart > memsz + sizeof(elf_module))return NULL;
    elf->vstart = vstart;
    uint32_t need_pg=ngx_align(need_sz,4096)/4096;
    elf->use_pgn=need_pg;
    //入口点位置计算
    newbase=kmalloc_page(need_pg);
    //printf("newbase:%x;",newbase);
    elf->elfdata = newbase;
    elf->entry = newbase + h->entry - vstart;    
    DEBUG_PRINTF("module: %d size: %d\n", elf, need_sz);
    DEBUG_PRINTF("  elf: %d\n", elf->elfdata);
    uint32_t ptr;
    //执行段拷贝到内存
    ph = (elf_program_header*)(elfdata + h->phoff);
    for(i = 0; i < h->phnum; i++){
        if(ph->type == ELF_PROGRAM_TYPE_LOAD){
            ptr = newbase + ph->vaddr - elf->vstart;
            psrc = (uint8_t*)elfdata + ph->offset;
            sz = ph->filesz;
            
            DEBUG_PRINTF("  load: %d-%d -> %d-%d size: %d\n", (void*)(uint32_t*)(psrc - elfdata), 
                (void*)(uint32_t*)(psrc - elfdata + sz), ptr, ptr + sz, (int)sz);
            memcpy(ptr, psrc, sz);
            ptr += sz;
            sz = ph->memsz - ph->filesz;
            if(sz){
                DEBUG_PRINTF("  bzero: %d-%d size: %d\n", ptr, ptr + sz, (int)sz);
                memset(ptr, 0, sz);  
            }          
        }
        ph ++; 
    }
    basesh = (elf_section_header*)(elfdata + h->shoff);
    sh = basesh;
    for(i = 0; i < h->shnum; i++){
        if(!strcmp(elf_solve_str(h,sh->name),".data")||!strcmp(elf_solve_str(h,sh->name),".rpdata")||!strcmp(elf_solve_str(h,sh->name),".bss"))
        {
            //printf("find %s section :0x%x sz:%d",elf_solve_str(h,sh->name),sh->addr,sh->size);
            //if(sh->addr+sh->size>vend)vend=sh->addr+sh->size;
            //printf("copy %s section from 0x%x to 0x%x;",elf_solve_str(h,sh->name),sh->offset,sh->addr);
            memcpy(newbase+sh->addr,elfdata+sh->offset,sh->size);
        }
        sh++;
    }
    //符号表位置计算
    basesh = (elf_section_header*)(elfdata + h->shoff);
    sh = basesh;
    elf->sym = NULL;
    elf_section_header *relocate_sec;
    for(i = 0; i < h->shnum; i++){
        
        if(sh->type == ELF_SECTION_TYPE_DYNSYM){
            elf->sym = (elf_symbol*)(newbase + sh->offset - psh->offset);
            elf->symnum = sh->size / sizeof(elf_symbol);
            elf->strtab = (char*)newbase + basesh[sh->link].offset - psh->offset;
            elf->strtabsz = basesh[sh->link].size;

            DEBUG_PRINTF("  dynsym: %d -> %d\n", (void*)sh->offset, elf->sym);
            DEBUG_PRINTF("  strtab: %d -> %d\n", (void*)basesh[sh->link].offset, elf->strtab);
        }else if(sh->type==ELF_SECTION_TYPE_REL)
        {
            elf_relocate(sh,elfdata,newbase);
            //printf("rel_section_name:%s;",elf_solve_str(h,sh->name));
            
        }
        sh ++; 
    }
    if(!elf->sym)return NULL;

    
    //DEBUG_PRINTF("  vstart: %x\n", (int)elf->vstart);    
    //DEBUG_PRINTF("  size: %x\n", (int)(vend - vstart));
    //DEBUG_PRINTF("  entry: %d\n", elf->entry);
    

    DEBUG_PRINTF("  symbols:\n");
    sym = elf->sym;    
    for(i = 0; i < elf->symnum; i++ ){
        DEBUG_PRINTF("  %d -> %d  %s\n", (void*)sym->value,  elf->elfdata + sym->value - elf->vstart , elf->strtab + sym->name);        
        sym ++;
    }
    return elf;
}

//获取入口函数地址
void *elf_module_entry(elf_module* m){
    return m->entry;
}

void elf_module_release(elf_module *m)
{
    kfree_page(m->elfdata,m->use_pgn);
    kfree(m);
}
/**
 * @brief WARNING
 * 
 * If you want to get a pointer 
 * Remember that you should use 2-level pointer 
 * (example, if you want to get a pointer p1 , you shoud use p1=*elf_module_sym(xxx,xxx)) 
 * 
 */
void *elf_module_sym(elf_module* m, const char* name){
    size_t i;
    elf_symbol* sym = NULL;

    sym = m->sym;    
    for(i = 0; i < m->symnum; i++ ){
        if(sym->name && sym->name < m->strtabsz && strcmp(m->strtab + sym->name, name) == 0){
            return m->elfdata + sym->value - m->vstart;
        }
        sym ++;
    } 

    return NULL;   
}























