#include"hardware/timer.h"
#include"gates/irq.h"
#include"console.h"
#include"io.h"
#include"process/task.h"
//execute n times per 1 second
#define T_UNIT 20000
uint32_t ticks=0;
static void timer_callback(registers_t regs)
{
    ticks++;
    if(get_running_progress()->time_left!=0){
		(get_running_progress()->time_left)--;
		(get_running_progress()->time_counter)++;
	}
	else{
        //printf("Schedule!Running Task ID : %d,Kernel Task:%d\n",get_running_progress()->tid,get_running_progress()->is_kern_thread);
		schedule();
	}
}
void reset_tick()
{
    ticks=0;
}
uint32_t get_tick()
{
    //printf("tick:%d",ticks);
    return ticks;
}
void ksleep(uint32_t ms)
{
    reset_tick();
    /**
     * @brief Why it seems so easy?
     * Because i simply set the TUNIT 1000
     * so 1000 per second
     * Guess what?
     * 1 tick = 1 ms 
     * 
     */
    //printf("<");
    while (get_tick()<ms)
    {
        
    }
    //printf(">");
    return;
}
void init_timer()
{
    register_interrupt_handler(IRQ0, &timer_callback);

   // The value we send to the PIT is the value to divide it's input clock
   // (1193180 Hz) by, to get our required frequency. Important to note is
   // that the divisor must be small enough to fit into 16-bits.
   uint32_t divisor = 1193180 / T_UNIT;

   // Send the command byte.
   outb(0x43, 0x36);

   // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
   uint8_t l = (uint8_t)(divisor & 0xFF);
   uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

   // Send the frequency divisor.
   outb(0x40, l);
   outb(0x40, h);
}