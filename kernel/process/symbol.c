#include"process/symbol.h"
#include"utils/hashmap.h"
#include"mem/malloc.h"
#include"string.h"
#include"kobjects/obj_vfs.h"
#include"mem/memorylayout.h"
#include"console.h"
#include"utils/qhash.h"
/* 
 * 将字符转换为数值 
 * */  
#define isdigit(c) ((c)>='0' && (c)<='9')
#define isalpha(c) ((c)>='A'&&(c)<='z')
#define isupper(c) ((c)>='A'&&(c)<='Z')
int c2i(char ch)  
{  
        // 如果是数字，则用数字的ASCII码减去48, 如果ch = '2' ,则 '2' - 48 = 2  
        if(isdigit(ch))  
                return ch - 48;  
  
        // 如果是字母，但不是A~F,a~f则返回  
        if( ch < 'A' || (ch > 'F' && ch < 'a') || ch > 'z' )  
                return -1;  
  
        // 如果是大写字母，则用数字的ASCII码减去55, 如果ch = 'A' ,则 'A' - 55 = 10  
        // 如果是小写字母，则用数字的ASCII码减去87, 如果ch = 'a' ,则 'a' - 87 = 10  
        if(isalpha(ch))  
                return isupper(ch) ? ch - 55 : ch - 87;  
  
        return -1;  
}  
  
/* 
 * 功能：将十六进制字符串转换为整型(int)数值 
 * */  
int hex2dec(char *hex)  
{  
        int len;  
        int num = 0;  
        int temp;  
        int bits;  
        int i;  
          
        // 此例中 hex = "1de" 长度为3, hex是main函数传递的  
        len = strlen(hex);  
  
        for (i=0, temp=0; i<len; i++, temp=0)  
        {  
                // 第一次：i=0, *(hex + i) = *(hex + 0) = '1', 即temp = 1  
                // 第二次：i=1, *(hex + i) = *(hex + 1) = 'd', 即temp = 13  
                // 第三次：i=2, *(hex + i) = *(hex + 2) = 'd', 即temp = 14  
                temp = c2i( *(hex + i) );  
                // 总共3位，一个16进制位用 4 bit保存  
                // 第一次：'1'为最高位，所以temp左移 (len - i -1) * 4 = 2 * 4 = 8 位  
                // 第二次：'d'为次高位，所以temp左移 (len - i -1) * 4 = 1 * 4 = 4 位  
                // 第三次：'e'为最低位，所以temp左移 (len - i -1) * 4 = 0 * 4 = 0 位  
                bits = (len - i - 1) * 4;  
                temp = temp << bits;  
  
                // 此处也可以用 num += temp;进行累加  
                num = num | temp;  
        }  
  
        // 返回结果  
        return num;  
}  
//map_t symbol_hash_map;
char *symbol_map;

typedef struct symbol_match
{
    uint32_t name_hash;
    uint32_t address;
}symbol_match_t;
symbol_match_t*sym_match_table;
uint32_t sym_len=0;
//static struct symbol_entry symbol_entries[SYMBOL_ENTRY_SLOTS];
//static int8_t symbol_map[SYMBOL_MAP_SIZE];

uint32_t symbol_find(const int8_t *name) {
    uint32_t h=get_qhash(name);
    for (int i = 0; i < sym_len; i++)
    {
        if(sym_match_table[i].name_hash==h)return sym_match_table[i].address;
    }
    
    return 0;
}

int symbol_init(void) {
    uint32_t count, i, start = 0, index = 0;
    int8_t *name, *address;
    
        //symbol_hash_map= hashmap_new();
    //if(!symbol_hash_map)return -1;
    int fd=sys_open("/boot/kernel.map",O_RDONLY);
    if(fd<0)return -1;
    sys_lseek(fd,0,SEEK_END);
    uint32_t sz=sys_tell(fd);
    printf("file sz:%d;",sz);
    symbol_map=kmalloc_page(ngx_align(sz,4096)/4096);
    sym_match_table=symbol_map;
    //symbol_match_t*cur_match=sym_match_table;
    if(!symbol_map)return -1;
    sys_lseek(fd,0,SEEK_SET);
    sys_read(fd,symbol_map,sz);
    //count = vfs_read("/ramdisk/boot/kernel.map", 0, SYMBOL_MAP_SIZE, symbol_map);
    //printf("%s",symbol_map);
    for (i = 0; i < sz; i++) {
        switch (symbol_map[i]) {
            case ' ':
                symbol_map[i] = '\0';
                break;
            case '\n':
                symbol_map[i] = '\0';
		/* TODO: I don't like this '+ 11' below */
		        name = symbol_map + start + 11;
		        address = symbol_map + start;
                sym_match_table[sym_len].address=hex2dec(address);
                sym_match_table[sym_len].name_hash=get_qhash(name);
                sym_len++;
                //hashmap_put(symbol_hash_map,name,address);
                
                //printf("map%s%s;",name,address);
                //memcpy(symbol_entries[index].name, name, strlen(name) + 1);
                //symbol_entries[index].paddress = strreadn(address, 16);
                index++;
                start = i + 1;
                break;
        }

    }
    printf("symlen:%d",sym_len);
    return 0;
}