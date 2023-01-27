/*
 * @Description: 键盘设备
 * @Author: QIUFUYU
 * @Date: 2021-10-01 21:13:00
 * @LastEditTime: 2022-01-10 22:19:36
 */
#include"hardware/keyboard/keyboard.h"
#include"types.h"
#include"gates/isr.h"
#include"gates/irq.h"
#include"process/sync.h"
#include"console.h"
#include"utils/circlequeue.h"
//ioqueue_t kbd_buf;
circlequeue_t key_board_queue;
circlequeue_t wait_thread_queue;
TCB_t* blocked_thread;
char current_key;
lock_t kbd_lock;
static inline uint8_t inb(uint16_t port) {
   uint8_t data;
   asm volatile ("inb %w1, %b0" : "=a" (data) : "Nd" (port));
   return data;
}
#define KBD_BUF_PORT 0x60	 // 键盘buffer寄存器端口号为0x60

/* 用转义字符定义部分控制字符 */
#define esc		'\033'	 // 八进制表示字符,也可以用十六进制'\x1b'
#define backspace	'\b'
#define tab		'\t'
#define enter		'\n'
#define delete		'\177'	 // 八进制表示字符,十六进制为'\x7f'

#define nullkey '\0'

/* 以上不可见字符一律定义为0 */
#define char_invisible	0
#define ctrl_l_char	char_invisible
#define ctrl_r_char	char_invisible
#define shift_l_char	char_invisible
#define shift_r_char	char_invisible
#define alt_l_char	char_invisible
#define alt_r_char	char_invisible
#define caps_lock_char	char_invisible

/* 定义控制字符的通码和断码 */
#define shift_l_make	0x2a
#define shift_r_make 	0x36 
#define alt_l_make   	0x38
#define alt_r_make   	0xe038
#define alt_r_break   	0xe0b8
#define ctrl_l_make  	0x1d
#define ctrl_r_make  	0xe01d
#define ctrl_r_break 	0xe09d
#define caps_lock_make 	0x3a
/* 定义以下变量记录相应键是否按下的状态,
 * ext_scancode用于记录makecode是否以0xe0开头 */
static bool ctrl_status, shift_status, alt_status, caps_lock_status, ext_scancode;

/*
举个例子，a的通码为0x1e，比如按下a键，若之前未按下shift键，
咱们应该处理为小写字符'a'，所以keymap[0x1e][0]等于'a'。
*/
/* 以通码make_code为索引的二维数组 */
static char keymap[][2] = {
/* 扫描码   未与shift组合  与shift组合*/
/* ---------------------------------- */
/* 0x00 */	{0,	0},		
/* 0x01 */	{esc,	esc},		
/* 0x02 */	{'1',	'!'},		
/* 0x03 */	{'2',	'@'},		
/* 0x04 */	{'3',	'#'},		
/* 0x05 */	{'4',	'$'},		
/* 0x06 */	{'5',	'%'},		
/* 0x07 */	{'6',	'^'},		
/* 0x08 */	{'7',	'&'},		
/* 0x09 */	{'8',	'*'},		
/* 0x0A */	{'9',	'('},		
/* 0x0B */	{'0',	')'},		
/* 0x0C */	{'-',	'_'},		
/* 0x0D */	{'=',	'+'},		
/* 0x0E */	{backspace, backspace},	
/* 0x0F */	{tab,	tab},		
/* 0x10 */	{'q',	'Q'},		
/* 0x11 */	{'w',	'W'},		
/* 0x12 */	{'e',	'E'},		
/* 0x13 */	{'r',	'R'},		
/* 0x14 */	{'t',	'T'},		
/* 0x15 */	{'y',	'Y'},		
/* 0x16 */	{'u',	'U'},		
/* 0x17 */	{'i',	'I'},		
/* 0x18 */	{'o',	'O'},		
/* 0x19 */	{'p',	'P'},		
/* 0x1A */	{'[',	'{'},		
/* 0x1B */	{']',	'}'},		
/* 0x1C */	{enter,  enter},
/* 0x1D */	{ctrl_l_char, ctrl_l_char},
/* 0x1E */	{'a',	'A'},		
/* 0x1F */	{'s',	'S'},		
/* 0x20 */	{'d',	'D'},		
/* 0x21 */	{'f',	'F'},		
/* 0x22 */	{'g',	'G'},		
/* 0x23 */	{'h',	'H'},		
/* 0x24 */	{'j',	'J'},		
/* 0x25 */	{'k',	'K'},		
/* 0x26 */	{'l',	'L'},		
/* 0x27 */	{';',	':'},		
/* 0x28 */	{'\'',	'"'},		
/* 0x29 */	{'`',	'~'},		
/* 0x2A */	{shift_l_char, shift_l_char},	
/* 0x2B */	{'\\',	'|'},		
/* 0x2C */	{'z',	'Z'},		
/* 0x2D */	{'x',	'X'},		
/* 0x2E */	{'c',	'C'},		
/* 0x2F */	{'v',	'V'},		
/* 0x30 */	{'b',	'B'},		
/* 0x31 */	{'n',	'N'},		
/* 0x32 */	{'m',	'M'},		
/* 0x33 */	{',',	'<'},		
/* 0x34 */	{'.',	'>'},		
/* 0x35 */	{'/',	'?'},
/* 0x36	*/	{shift_r_char, shift_r_char},	
/* 0x37 */	{'*',	'*'},    	
/* 0x38 */	{alt_l_char, alt_l_char},
/* 0x39 */	{' ',	' '},		
/* 0x3A */	{caps_lock_char, caps_lock_char},
/* F1 */    {nullkey,nullkey},
/* F2 */    {nullkey,nullkey},
/* F3 */    {nullkey,nullkey},
/* F4 */    {nullkey,nullkey},
/* F5 */    {nullkey,nullkey},
/* F6 */    {nullkey,nullkey},
/* F7 */    {nullkey,nullkey},
/* F8 */    {nullkey,nullkey},
/* F9 */    {nullkey,nullkey},
/* F10 0x44*/    {nullkey,nullkey},
/* numlock 0x45*/{nullkey,nullkey},
/* scroll lock 0x46*/{nullkey,nullkey},
{nullkey,nullkey},
{nullkey,nullkey},
/*0x49*/{nullkey,nullkey},
/*0x4a*/{nullkey,nullkey},
/*0x4b*/{nullkey,nullkey},
/*0x4c*/{nullkey,nullkey},
/*0x4d*/{nullkey,nullkey},
/*0x4e*/{nullkey,nullkey},
/*0x4f*/{nullkey,nullkey},
/*0x50*/{nullkey,nullkey}

/*其它按键暂不处理*/
};

/* 键盘中断处理程序 
键盘中断处理程序始终是每次处理一个字节，所以当扫描码中是多字节时，
或者有组合键时，咱们要定义额外的全局变量来记录它们曾经被按下过。
*/
void intr_keyboard_handler(registers_t reg) {
	//printf("key down!");
   //console_print_str("key !\n");
/* 这次中断发生前的上一次中断,以下任意三个键是否有按下 */
   bool ctrl_down_last = ctrl_status;	  
   bool shift_down_last = shift_status;
   bool caps_lock_last = caps_lock_status;

   bool break_code;
   uint16_t scancode = inb(KBD_BUF_PORT);

/* 若扫描码是e0开头的,表示此键的按下将产生多个扫描码,
 * 所以马上结束此次中断处理函数,等待下一个扫描码进来*/ 
   if (scancode == 0xe0) { 
      ext_scancode = true;    // 打开e0标记
      return;
   }

/* 如果上次是以0xe0开头,将扫描码合并 */
   if (ext_scancode) {
      scancode = ((0xe000) | scancode);
      ext_scancode = false;   // 关闭e0标记
   }   

   break_code = ((scancode & 0x0080) != 0);   // 获取break_code 断码的第8位为1 0b10000000
   
   if (break_code) {   // 若是断码break_code(按键弹起时产生的扫描码)

   /* 由于ctrl_r 和alt_r的make_code和break_code都是两字节,
   所以可用下面的方法取make_code,多字节的扫描码暂不处理 */
   // 断码的第8位为1，通码的第8位为0 扫描码scancode（此时它为断码）与0xff7f进行位与运算，抹去第8位的1，这样就获得了其通码
      uint8_t make_code = (scancode &= 0xff7f);   // 得到其make_code(按键按下时产生的扫描码)

   /* 判断此通码是否为ctrl、shift、alt 若是任意以下三个键弹起了,将状态置为false */
      if (make_code == ctrl_l_make || make_code == ctrl_r_make) {
	 ctrl_status = false;
      } else if (make_code == shift_l_make || make_code == shift_r_make) {
	 shift_status = false;
      } else if (make_code == alt_l_make || make_code == alt_r_make) {
	 alt_status = false;
      } /* 由于caps_lock不是弹起后关闭,所以需要单独处理 */

      return;   // 直接返回结束此次中断处理程序

   } 
   /* 若为通码,只处理数组中定义的键以及alt_right和ctrl键,全是make_code */
   else if ((scancode > 0x00 && scancode < 0x3b) || \
	       (scancode == alt_r_make) || \
	       (scancode == ctrl_r_make)) {
      bool shift = false;  // 判断是否与shift组合,用来在一维数组中索引对应的字符
      if ((scancode < 0x0e) || (scancode == 0x29) || \
	 (scancode == 0x1a) || (scancode == 0x1b) || \
	 (scancode == 0x2b) || (scancode == 0x27) || \
	 (scancode == 0x28) || (scancode == 0x33) || \
	 (scancode == 0x34) || (scancode == 0x35)) {  // 先处理双字符键
	    /****** 代表两个字母的键 ********
		     0x0e 数字'0'~'9',字符'-',字符'='
		     0x29 字符'`'
		     0x1a 字符'['
		     0x1b 字符']'
		     0x2b 字符'\\'
		     0x27 字符';'
		     0x28 字符'\''
		     0x33 字符','
		     0x34 字符'.'
		     0x35 字符'/' 
	    *******************************/
	 if (shift_down_last) {  // 如果同时按下了shift键
	    shift = true;
	   }
   } else {	  // 默认为字母键 处理字母键
	 if (shift_down_last && caps_lock_last) {  // 如果shift和capslock同时按下 抵销了大写功能
	    shift = false;
	 } else if (shift_down_last || caps_lock_last) { // 如果shift和capslock任意被按下 如果<shift>和<caps lock>仅按下了一个，大写功能开启
	    shift = true;
	 } else {
	    shift = false;
	 }
      }
      // 有可能此扫描码是2字节，高字节是0xe0，我们要抹去它
      uint8_t mark=(scancode & 0xff00)>>8;
      uint8_t index = (scancode &= 0x00ff);  // 将扫描码的高字节置0,主要是针对高字节是e0的扫描码.
      char cur_char;
      if(!mark)
         cur_char = keymap[index][shift];  // 在数组中找到对应的字符
      else if(mark==0xe0)
      {
         if(index==0x48)
         {
            cur_char=KEY_PGUP;
            printf("PG_UP");
         }
      }
      /* 只处理ascii码不为0的键 */
      /*如果cur_char为0，根据目前keymap的定义，
      表示它们是操作控制键<ctrl>、<shift>、<alt>
      或<capslock>之一，只有这4个按键对应的ASCII码
      为0，注意，现在处于通码的代码块中，因此要判断下
      是它们中的哪一个按下了。*/
      if (cur_char) {
         if(!circlequeue_full(&key_board_queue))
         {
	         //put_char(cur_char,BLACK,WHITE);
			 //printf("[%c]",cur_char);
            current_key=cur_char;
            if(mark)
            {
               circlequeue_push(&key_board_queue,"\0");
            }
            circlequeue_push(&key_board_queue,&cur_char);
            thread_wakeup(blocked_thread);
         }
		//printf("%c",cur_char);
	 	return;
      }

      /* 记录本次是否按下了下面几类控制键之一,供下次键入时判断组合键 */
      if (scancode == ctrl_l_make || scancode == ctrl_r_make) {
	 ctrl_status = true;
      } else if (scancode == shift_l_make || scancode == shift_r_make) {
	 shift_status = true;
      } else if (scancode == alt_l_make || scancode == alt_r_make) {
	 alt_status = true;
      } else if (scancode == caps_lock_make) {
      /* 不管之前是否有按下caps_lock键,当再次按下时则状态取反,
       * 即:已经开启时,再按下同样的键是关闭。关闭时按下表示开启。*/
	 caps_lock_status = !caps_lock_status;
      }
   } else {
         uint8_t mark=(scancode & 0xff00)>>8;
         uint8_t index = (scancode &= 0x00ff);  // 将扫描码的高字节置0,主要是针对高字节是e0的扫描码.
         char cur_char;
         if(!mark)
            cur_char = index;  // 在数组中找到对应的字符
         else if(mark==0xe0)
         {

            if(index==0x49 || index==0x48)
               index=KEY_PGUP;
            else if(index==0x51 || index==0x50)
               index=KEY_PGDOWN;
            else if(index==0x4d)
               index=KEY_RIGHT;
            else if(index==0x4b)
               index=KEY_LEFT;
         }
         if(!circlequeue_full(&key_board_queue))
         {
	         //put_char(cur_char,BLACK,WHITE);
            current_key=index;
            if(mark)
            {
               circlequeue_push(&key_board_queue,"\0");
            }
            circlequeue_push(&key_board_queue,&index);
            thread_wakeup(blocked_thread);
         }
      //console_print_str("unknow key ");
   }
}

/* 键盘初始化 */
void keyboard_init() {
   //ioqueue_init(&kbd_buf,1,60);
   //put_str("keyboard init start\n");
   lock_init(&kbd_lock);
   printf("kbdinit");
   circlequeue_init(&key_board_queue,128,1);
   circlequeue_init(&wait_thread_queue,20,4);
   register_interrupt_handler(0x21,&intr_keyboard_handler);
   //register_handler(0x21, intr_keyboard_handler);
   //put_str("keyboard init done\n");
}
char keyboard_get_key()
{
	lock_acquire(&kbd_lock);
	char k=0;
	if(!circlequeue_empty(&key_board_queue))
	{
		k=*(char*)circlequeue_get(&key_board_queue);
	}else
   {
      blocked_thread=get_running_progress();
      //circlequeue_push(&wait_thread_queue,get_running_progress());
      //printf("[kbd block]");
      thread_block();
      //printf("[kbd wakeup]");
      k=*(char*)circlequeue_get(&key_board_queue);
   }

	lock_release(&kbd_lock);
	return k;
        //enum intr_status old=intr_disable();
        // intr_disable_loop();
        // //intr_disable();
        // if(!is_empty(&kbd_buf))
        // {
        //     char *byte=ioq_get(&kbd_buf);
        //     //intr_set_status(old);
        //     intr_enable_loop();
        //     return *byte;
        // }
        // intr_enable_loop();
        //intr_enable();
        //intr_set_status(old);
       // return 0;
}