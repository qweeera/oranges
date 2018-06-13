#include "proto.h"
#include "i8259.h"
#include "proc.h" 
void init_clock(void)
{
	 /* 初始化 8253 PIT 10ms 中断一次 */
        out_byte(TIMER_MODE, RATE_GENERATOR);
        out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
        out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

	/* 时钟中断使能 */
	put_irq_handler(CLOCK_IRQ, clock_handler); //设定时钟中断处理程序
        enable_irq(CLOCK_IRQ);
}

void clock_handler(int irq)
{
//	disp_str("#");
	ticks++;

	p_proc_ready->ticks--;  //当前进程的时间片减一

	if(k_reenter != 0)
	{
//		disp_str("!");
		return ;
	}

	if(p_proc_ready->ticks > 0)
		return;

	schedule();

}
