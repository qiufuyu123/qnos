#include"console.h"
#include"KMDK/KMDK.h"
#include"process/task.h"
#include"process/syscall.h"
#include"sysmodule.h"
char *a_str="This is a string stored in .data section";
//const char *b_str="Test Module"

int kread(int idx,char *buffer,uint32_t size,uint32_t flag)
{
    
}
int kwrite(int idx,char *buffer,uint32_t size,uint32_t flag)
{
    
}

KM_CREATE(km_info){
    .description="This is a string stored in .data section",
    .name="Test Module",
    .versions=KM_MAKE_VERSION(1,0,0),
    .ops.read=kread,
    .ops.write=kwrite
};
//uint32_t __QNOS_SYSM_NAME=1;
//static int pv2;
int user_main_thread(int argc,char **argv)
{
    __base_syscall(0,233,322,232,323);
    return 233;
}
int release()
{

}
int main()
{
    printf("Welcome to the main thread! %d:%s",get_running_progress()->tid,get_running_progress()->name);
    while(1);
}
int init( sysmodule_t*m)
{
    //pv2 =2;
    //__QNOS_SYSM_NAME+=4;
    printf("sysmodule:%s has loaded!; bind kobj:%s",m->km_info->name,m->bind_obj->name);
    Klogger->putstr("KLogg");
    return a_str;
}
//KMDKInfo_t* a=&km_info;
KM_RELEASE(km_info)