
#include"hardware/mouse.h"
#include"io.h"
#include"console.h"
#include"gates/irq.h"
#include"string.h"
/* 键盘控制器端口 */
enum KeyboardControllerPorts {
    KBC_READ_DATA   = 0x60,     /* 读取数据端口(R) */
    KBC_WRITE_DATA  = 0x60,     /* 写入数据端口(W) */
    KBC_STATUS      = 0x64,     /* 获取控制器状态(R) */
    KBC_CMD         = 0x64,     /* 向控制器发送命令(W) */
};

/* 键盘控制器的命令 */
enum KeyboardControllerCmds {
    KBC_CMD_READ_CONFIG     = 0x20,     /* 读取配置命令 */
    KBC_CMD_WRITE_CONFIG    = 0x60,     /* 写入配置命令 */
    KBC_CMD_DISABLE_MOUSE   = 0xA7,     /* 禁止鼠标端口 */
    KBC_CMD_ENABLE_MOUSE    = 0xA8,     /* 开启鼠标端口 */
    KBC_CMD_DISABLE_KEY     = 0xA7,     /* 禁止键盘通信，自动复位控制器状态的第4位 */
    KBC_CMD_ENABLE_KEY      = 0xA8,     /* 开启键盘通信，自动置位控制器状态的第4位 */
    KBC_CMD_SEND_TO_MOUSE   = 0xD4,     /* 向鼠标发送数据 */
    KBC_CMD_REBOOT_SYSTEM   = 0xFE,     /* 系统重启 */    
};

/* 键盘配置位 */
enum KeyboardControllerConfigBits {
    KBC_CFG_ENABLE_KEY_INTR     = (1 << 0), /* bit 0=1: 使能键盘中断IRQ1(IBE) */
    KBC_CFG_ENABLE_MOUSE_INTR   = (1 << 1), /* bit 1=1: 使能鼠标中断IRQ12(MIBE) */
    KBC_CFG_INIT_DONE           = (1 << 2), /* bit 2=1: 设置状态寄存器的位2 */
    KBC_CFG_IGNORE_STATUS_BIT4  = (1 << 3), /* bit 3=1: 忽略状态寄存器中的位4 */
    KBC_CFG_DISABLE_KEY         = (1 << 4), /* bit 4=1: 禁止键盘 */
    KBC_CFG_DISABLE_MOUSE       = (1 << 5), /* bit 5=1: 禁止鼠标 */
    KBC_CFG_SCAN_CODE_TRANS     = (1 << 6), /* bit 6=1: 将第二套扫描码翻译为第一套 */
    /* bit 7 保留为0 */
};

/* 键盘控制器状态位 */
enum KeyboardControllerStatusBits {
    KBC_STATUS_OUT_BUF_FULL     = (1 << 0), /* OUT_BUF_FULL: 输出缓冲器满置1，CPU读取后置0 */
    KBC_STATUS_INPUT_BUF_FULL   = (1 << 1), /* INPUT_BUF_FULL: 输入缓冲器满置1，i8042 取走后置0 */
    KBC_STATUS_SYS_FLAG         = (1 << 2), /* SYS_FLAG: 系统标志，加电启动置0，自检通过后置1 */
    KBC_STATUS_CMD_DATA         = (1 << 3), /* CMD_DATA: 为1，输入缓冲器中的内容为命令，为0，输入缓冲器中的内容为数据。 */
    KBC_STATUS_KYBD_INH         = (1 << 4), /* KYBD_INH: 为1，键盘没有被禁止。为0，键盘被禁止。 */
    KBC_STATUS_TRANS_TMOUT      = (1 << 5), /* TRANS_TMOUT: 发送超时，置1 */
    KBC_STATUS_RCV_TMOUT        = (1 << 6), /* RCV-TMOUT: 接收超时，置1 */
    KBC_STATUS_PARITY_EVEN      = (1 << 7), /* PARITY-EVEN: 从键盘获得的数据奇偶校验错误 */
};

/* 键盘控制器发送命令后 */
enum KeyboardControllerReturnCode {
    /* 当击键或释放键时检测到错误时，则在Output Bufer后放入此字节，
    如果Output Buffer已满，则会将Output Buffer的最后一个字节替代为此字节。
    使用Scan code set 1时使用00h，Scan code 2和Scan Code 3使用FFh。 */
    KBC_RET_KEY_ERROR_00    = 0x00,
    KBC_RET_KEY_ERROR_FF    = 0xFF,
    
    /* AAH, BAT完成代码。如果键盘检测成功，则会将此字节发送到8042 Output Register中。 */
    KBC_RET_BAT_OK          = 0xAA,

    /* EEH, Echo响应。Keyboard使用EEh响应从60h发来的Echo请求。 */
    KBC_RET_ECHO            = 0xEE,

    /* F0H, 在Scan code set 2和Scan code set 3中，被用作Break Code的前缀。*/
    KBC_RET_BREAK           = 0xF0,
    /* FAH, ACK。当Keyboard任何时候收到一个来自于60h端口的合法命令或合法数据之后，
    都回复一个FAh。 */
    KBC_RET_ACK             = 0xFA,
    
    /* FCH, BAT失败代码。如果键盘检测失败，则会将此字节发送到8042 Output Register中。 */
    KBC_RET_BAT_BAD         = 0xFC,

    /* FEH, 当Keyboard任何时候收到一个来自于60h端口的非法命令或非法数据之后，
    或者数据的奇偶交验错误，都回复一个FEh，要求系统重新发送相关命令或数据。 */
    KBC_RET_RESEND          = 0xFE,
};

/* 单独发送给鼠标的命令，有别于键盘控制器命令
首先得在命令端口0x64发送一个命令，把数据发送到鼠标的命令，0xD4，
这样0x60中的数据才是发送到鼠标，不然就是发送到键盘的。
这些命令是发送到数据端口0x60，而不是命令0x64，
如果有参数，就在发送一次到0x60即可。
 */
enum MouseCmds {
    /*
    大部分鼠标设备的ID号为0，部分鼠标根据设备ID号会有不同的数据包，
    ID为3时是3B数据包，ID为4时是4B两张数据包。
    */
    MOUSE_CMD_GET_DEVICE_ID =  0xF2,   /* 获取键盘设备的ID号 */                
    MOUSE_CMD_ENABLE_SEND   =  0xF4,   /* 允许鼠标设备发送数据包 */
    MOUSE_CMD_DISABLE_SEND  =  0xF5,   /* 禁止鼠标设备发送数据包 */
    MOUSE_CMD_RESTART       =  0xFF,   /* 重启鼠标 */
};

/* 键盘控制器配置 */
#define KBC_CONFIG	(KBC_CFG_ENABLE_KEY_INTR | KBC_CFG_ENABLE_MOUSE_INTR | \
                    KBC_CFG_INIT_DONE | KBC_CFG_SCAN_CODE_TRANS)

/* 等待键盘控制器可写入，当输入缓冲区为空后才可以写入 */
#define WAIT_KBC_WRITE()    while (inb(KBC_STATUS) & KBC_STATUS_INPUT_BUF_FULL)
/* 等待键盘控制器可读取，当输出缓冲区为空后才可以读取 */
#define WAIT_KBC_READ()    while (inb(KBC_STATUS) & KBC_STATUS_OUT_BUF_FULL)

/* 鼠标数据包 */
struct MouseData {
	/* 第一字节数据内容
	7: y overflow, 6: x overflow, 5: y sign bit, 4: x sign bit,
	3: alawys 1  , 2: middle btn, 1: right btn , 0: left btn  .
	*/
	unsigned char byte0;
	unsigned char byte1;		// x 移动值
	unsigned char byte2;		// y 移动值
	unsigned char byte3;		// z 移动值
	/*
	7: y always 0, 6: always 0, 5: 鼠标第5键, 4: 鼠标第4键,
	3: 滚轮水平左滚动, 2: 滚轮水平右滚动, 1: 滚轮垂直下滚动, 0: 滚轮垂直上滚动.
	*/
	unsigned char byte4;
};

/* 16个鼠标缓冲区 */
#define MOUSE_BUFFER_LEN  16

/*
键盘的私有数据
*/
struct MousePrivate {
	struct MouseData mouseData;		                        /* 鼠标的数据包 */
	char phase;		                                        /* 解析步骤 */
    uint8_t rowData;                                        /* 原始数据 */
    
	//InputDevice_t iptdev;   /* 输入设备 */
};
/* 输入缓冲区之鼠标 */
typedef struct InputBufferMouse {
    //InputBufferType_t type;
    uint8_t button;     /*  按钮 */
    int16_t xIncrease; /* 水平增长 */
    int16_t yIncrease; /* 垂直增长 */
    int16_t zIncrease; /* 纵深增长 */
} InputBufferMouse_t;

/* 缓冲区类型输入设备鼠标数据模型 */
typedef union InputBuffer {
    //InputBufferType_t type;
    InputBufferMouse_t mouse;
} InputBuffer_t;
/* 键盘的私有数据 */
struct MousePrivate mousePrivate;

/* 等待键盘控制器应答，如果不是回复码就一直等待
这个本应该是宏的，但是在vmware虚拟机中会卡在那儿，所以改成宏类函数
 */
void WAIT_KBC_ACK()
{
	unsigned char read;
	do {
		read = inb(KBC_READ_DATA);
	} while ((read =! KBC_RET_ACK));
}

/**
 * MouseDoRead - 解析鼠标数据
 * 
 * 返回-1，表明解析出错。
 * 返回0，表明解析成功，但还没有获取完整的数据包
 * 返回1，表明解析成功，获取了完整的数据包
*/
int MouseDoRead()
{
    unsigned char rowData = mousePrivate.rowData;
	
    if (mousePrivate.phase == 0) {
        /* 打开中断后产生的第一个数据是ACK码，然后才开始数据传输 */
		//if (rowData == KBC_RET_ACK) {
			mousePrivate.phase++;
        //}
		return 0;
	}
	if (mousePrivate.phase == 1) {
			mousePrivate.mouseData.byte0 = rowData;
			mousePrivate.phase++;
		return 0;
	}
	if (mousePrivate.phase == 2) {
		mousePrivate.mouseData.byte1 = rowData;
		mousePrivate.phase++;
		return 0;
	}
	if (mousePrivate.phase == 3) {
		mousePrivate.mouseData.byte2 = rowData;
		mousePrivate.phase = 1;

		//printk("(B:%x, X:%d, Y:%d)\n", mousePrivate.mouseData.byte0, (char)mousePrivate.mouseData.byte1, (char)mousePrivate.mouseData.byte2);
        /* 开始数据包的填写 */
        InputBuffer_t buffer;
        //buffer.type = INPUT_BUFFER_MOUSE;
        /* 只获取低3位，也就是按键按下的位 */
        buffer.mouse.button = mousePrivate.mouseData.byte0 & 0x07;
        buffer.mouse.xIncrease = mousePrivate.mouseData.byte1;
        buffer.mouse.yIncrease = mousePrivate.mouseData.byte2;
        /* 如果x有符号，那么就添加上符号 */
        if ((mousePrivate.mouseData.byte0 & 0x10) != 0)
            buffer.mouse.xIncrease |= 0xff00;
        /* 如果y有符号，那么就添加上符号 */
        if ((mousePrivate.mouseData.byte0 & 0x20) != 0)
            buffer.mouse.yIncrease |= 0xff00;
        /* y增长是反向的，所以要取反 */
        buffer.mouse.yIncrease = -buffer.mouse.yIncrease;
        buffer.mouse.zIncrease = mousePrivate.mouseData.byte3;
        
        //printf("[x:%d, y:%d](%x)\n", buffer.mouse.xIncrease, buffer.mouse.yIncrease, buffer.mouse.button);
        //InputDevicePutBuffer(&mousePrivate.iptdev, &buffer);
		return 1;
	}
	return -1; 
}

/**
 * MouseHnadler - 鼠标中断处理函数
 * @irq: 中断号
 * @data: 中断的数据
 */
void MouseHnadler(registers_t regs)
{
    
    uint8_t rowData = inb(KBC_READ_DATA);
    mousePrivate.rowData=rowData;
    /* 直接处理数据 */
    MouseDoRead();
}
int kmouse_read(kdevice_t*self,uint32_t addr,uint32_t nums,char *buffer, int flag)
{
    if(nums!=sizeof(InputBufferMouse_t))return -1;
    memcpy(buffer,&mousePrivate.mouseData,sizeof(InputBufferMouse_t));   
    return sizeof(InputBufferMouse_t);
}
int kmouse_write(kdevice_t*self,uint32_t addr,uint32_t nums,char *buffer, int flag)
{
    return -1;
}
int kmouse_mmap(struct kdevice*self, void*starts,uint32_t length,int offset,int flag)
{
    return -1;
}
kdevice_ops_t kmouse_ops={
    .read=kmouse_read,
    .write=kmouse_write,
    .mmap=kmouse_mmap
};
/**
 * InitPs2MouseDriver - 初始化鼠标驱动
 */
int InitPs2MouseDriver()
{
    /* 注册输入设备 */
    register_interrupt_handler(IRQ12,&MouseHnadler);
    
	/* 初始化私有数据 */
	memset(&mousePrivate.mouseData, 0, sizeof(struct MouseData));

	mousePrivate.rowData = 0;
    mousePrivate.phase = 0;
    
    
    /* 开启鼠标端口 */
    WAIT_KBC_WRITE();
	outb(KBC_CMD, KBC_CMD_ENABLE_MOUSE);
	WAIT_KBC_ACK();
	
	/* 设置发送数据给鼠标 */
	WAIT_KBC_WRITE();
	outb(KBC_CMD, KBC_CMD_SEND_TO_MOUSE);
	WAIT_KBC_ACK();
	
	/* 传递打开鼠标的数据传输 */
	WAIT_KBC_WRITE();
	outb(KBC_WRITE_DATA, MOUSE_CMD_ENABLE_SEND);
	WAIT_KBC_ACK();
	    
    /* 设置鼠标与键盘使能以及IRQ中断开启 */
    WAIT_KBC_WRITE();
	outb(KBC_CMD, KBC_CMD_WRITE_CONFIG);
	WAIT_KBC_ACK();
	
    /* 配置键盘控制器的值 */
    WAIT_KBC_WRITE();
	outb(KBC_WRITE_DATA, KBC_CONFIG);
	WAIT_KBC_ACK();

    printf("init mouse done!\n");
    return 0;
}

kdevice_t* device_create_kmouse()
{
    InitPs2MouseDriver();
    return device_create("ps2",KDEV_BLOCK,KDEV_MOUSE,0,0,sizeof(InputBufferMouse_t),&kmouse_ops,0,0);
}