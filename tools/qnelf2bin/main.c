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
uint32_t ELF_get_entry_vaddr(Elf32_Ehdr* ehdr) {
    //printf("Ccc");
    fseek(fp, 0, SEEK_SET);
    fread(ehdr, sizeof(Elf32_Ehdr), 1, fp);
    return ehdr->e_entry;
    
}

typedef struct 
{
    char magic[3];
    uint32_t v_entry;
}QNBinary_t;

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
    uint32_t vaddr=ELF_get_entry_vaddr(&ehdr);
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
    QNBinary_t bhead;
    bhead.magic[0]='Q';bhead.magic[1]='B';bhead.magic[2]='F';
    bhead.v_entry=vaddr;
    fclose(fp);
    fp=fopen(foutname,"wb");
    if(!fp){ERR_OUT(printf("Fail to create destination file %s",foutname))}
    fwrite(&bhead,sizeof(QNBinary_t),1,fp);
    fwrite(fbuf,lens,1,fp);
    fclose(fp);
    return 0;
}