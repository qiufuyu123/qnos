#include"hardware/kbd.h"
#include"hardware/keyboard/keyboard.h"
extern char current_key;
int kbd_read(kdevice_t*self,uint32_t addr,uint32_t num,char *buffer,int flag)
{
    *buffer=current_key;
    return 1;
}
kdevice_ops_t kbd_ops={
    .read=kbd_read
};
kdevice_t* device_create_kbd(int id)
{
    return device_create("kbd0",KDEV_CHAR,KDEV_KBD,0,0,1,&kbd_ops,0,0);
}