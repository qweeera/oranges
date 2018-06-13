#include "string.h"
#include "protect.h"


void TestA();
void TestB();
void TestC();

TASK task_table[NR_TASKS] = { {TestA, STACK_SIZE_TESTA, "TestA"},
			      {TestB, STACK_SIZE_TESTB, "TestB"},
			      {TestC, STACK_SIZE_TESTC, "TestC"}};

int ticks = 0;


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

//	p_proc_ready++;
//	if(p_proc_ready >= proc_table + NR_TASKS)
//		p_proc_ready = proc_table;
}

void  milli_delay(int milli_sec)
{
	int t = get_ticks();
	while(((get_ticks() - t) * 1000 / HZ) < milli_sec){}
}



/*======================================================================*
                            kernel_main
 *======================================================================*/
int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*	p_task	= task_table;
	PROCESS* p_proc	= proc_table;
	char*	p_task_stack = task_stack +STACK_SIZE_TOTAL;
	u16	selector_ldt = SELECTOR_LDT_FIRST;
	int i;
	for(i=0; i<NR_TASKS; i++)
	{
		strcpy(p_proc->p_name, p_task->name);
		p_proc->pid	= i;
		p_proc->ldt_sel	= selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS>>3], sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;	// change the DPL
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS>>3], sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;	// change the DPL

		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;
		p_proc->regs.eip= (u32)p_task->initial_eip;
		p_proc->regs.esp= (u32) p_task_stack;
		p_proc->regs.eflags = 0x1202;	// IF=1, IOPL=1, bit 2 is always 1.

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += (1<<3);
	}

	proc_table[0].ticks = proc_table[0].priority = 150;
	proc_table[1].ticks = proc_table[1].priority =  50;
	proc_table[2].ticks = proc_table[2].priority =  30;

	k_reenter	= 0;

	p_proc_ready	= proc_table;  //还是从进程1开始

	/* 初始化 8253 PIT 10ms 中断一次 */
	out_byte(TIMER_MODE, RATE_GENERATOR);
        out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
        out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler); //设定时钟中断处理程序
	enable_irq(CLOCK_IRQ);


	restart();

	while(1){}
}


/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	int i = 0;
	while(1){

		disp_str("A.");
//		disp_int(get_ticks());
		milli_delay(200);
	}
}


void TestB()
{
	while(1){
		disp_str("B.");
//		disp_int(get_ticks());
		milli_delay(200);
	}
}

void TestC()
{
	while(1){
		disp_str("C.");
//		disp_int(get_ticks());
		milli_delay(200);
	}
}
