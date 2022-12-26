#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <string.h>
#include<unistd.h>
#define putline puts("-------------------------------------------------------------------------------------------------------------")
#define ERR_OUT(func) func;exit(-1);
int OStype;
FILE *fp;
char *fname,*foutname;
char cmds[100];
typedef struct 
{
    char magic[3];
    uint32_t v_entry;
    uint32_t v_loads[5][2];
}QNBinary_t;
uint32_t ELF_get_entry_vaddr(Elf32_Ehdr* ehdr) {
    //printf("Ccc");
    fseek(fp, 0, SEEK_SET);
    fread(ehdr, sizeof(Elf32_Ehdr), 1, fp);
    return ehdr->e_entry;
    
}
#define ngx_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
char strtable[9999];

int program_header_32_parse(Elf32_Ehdr* ehdr,QNBinary_t *b) {
    Elf32_Phdr phdr[99];
    fseek(fp, ehdr->e_phoff, SEEK_SET);
    int count = ehdr->e_phnum;    //程序头表的数量
    fread(phdr, sizeof(Elf32_Phdr), count, fp);
    putline;
    for(int i = 0; i < count; ++i) {
        switch(phdr[i].p_type) {
            //case 0 : printf("PT_NULL\t"); break;
            case 1 :  
            if(i<5)
            {
                if(i==0)break;
                b->v_loads[i][0]=phdr[i].p_vaddr;
                b->v_loads[i][1]=ngx_align(phdr[i].p_memsz,4096)/4096;
                printf("%x-->%x\n",b->v_loads[i][0],b->v_loads[i][1]);
            }else return -1;
            break;
        }
        
        
    }
    return 1;
}


int main(int argc,char *argv[]) {
    
    //return 0;
    char str[30];
    if(argc<3){ERR_OUT(printf("Need at least 2 args!"));}
    fname=argv[1];
    foutname=argv[2];
    //printf("aaa");
    //memcpy(fname,argv[1],strlen(argv[1]));
    //printf("file%s\n",fname);
    //fname = argv[1];
    fp = fopen(fname, "rb");
    if(fp == NULL) {
        printf("%s not exit\n", fname);
        exit(1);
    }
    //printf("f open ok!");
    memset(str, 0, sizeof(str));
    fread(str, 1, 5, fp);
    if(str[0] != 0x7f || str[1] != 'E' || str[2] != 'L' || str[3] != 'F') {
        ERR_OUT(printf("%s is not an ELF file!",argv[1]))
    }
    Elf32_Ehdr ehdr;
    //printf("a");
    QNBinary_t bhead;
    for(int i=0;i<5;i++)
    {
        bhead.v_loads[i][0]=bhead.v_loads[i][1]=0;
    }
    uint32_t vaddr=ELF_get_entry_vaddr(&ehdr);
    if(program_header_32_parse(&ehdr,&bhead)==-1)
    {
        printf("TOO MUCH LOAD HEADS!\n");
        return -1;
    }
    //printf("b");
    //printf("entry vaddr:0x%x %s\n",vaddr,fname);
    sprintf(cmds,"objcopy -O binary %s %s.obj",fname,fname);
    //printf("cmds:%s",cmds);
    system(cmds);
    memset(cmds,0,100);
    fclose(fp);
    sprintf(cmds,"%s.obj",fname);
    //printf("fname:%s;cmd%s",fname,cmds);
    fp=fopen(cmds,"rb");
    if(!fp){ERR_OUT(printf("Fail to convert into binary by command '%s'!",cmds))}
    fseek(fp,0,SEEK_END);
    uint32_t lens=ftell(fp);
    char *fbuf=malloc(lens+1);
    fseek(fp,0,SEEK_SET);
    fread(fbuf,lens,1,fp);
    
    bhead.magic[0]='Q';bhead.magic[1]='B';bhead.magic[2]='F';
    bhead.v_entry=vaddr;
    fclose(fp);
    fp=fopen(foutname,"wb");
    if(!fp){ERR_OUT(printf("Fail to create destination file %s",foutname))}
    fwrite(&bhead,sizeof(QNBinary_t),1,fp);
    fwrite(fbuf,lens,1,fp);
    fclose(fp);
    sprintf(cmds,"rm %s.obj -rf",fname);
    system(cmds);
    return 0;
}