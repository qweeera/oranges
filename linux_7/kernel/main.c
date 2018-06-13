#include "string.h"
#include "protect.h"
#include "console.h"
#include "proc.h"

void TestA();
void TestB();
void TestC();

TASK task_table[NR_TASKS] = { 
			      {task_tty, STACK_SIZE_TTY, "TTY"}};

TASK user_proc_table[NR_PROCS] = {
			      	   {TestA, STACK_SIZE_TESTA, "TestA"},
			           {TestB, STACK_SIZE_TESTB, "TestB"},
			           {TestC, STACK_SIZE_TESTC, "TestC"}};

int ticks = 0;




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
	u8	privilege;
	u8	rpl;
	int	eflags;
	for(i=0; i<NR_TASKS + NR_PROCS; i++)
	{
		if(i < NR_TASKS){    /*任务*/
			p_task 		= task_table + i;
			privilege	= PRIVILEGE_TASK;
			rpl		= RPL_TASK;
			eflags		= 0x1202;
		}
		else{
			p_task 		= user_proc_table + (i - NR_TASKS);
			privilege	= PRIVILEGE_USER;
			rpl		= RPL_USER;
			eflags		= 0x202;
		}
			
		strcpy(p_proc->p_name, p_task->name);
		p_proc->pid	= i;
		p_proc->ldt_sel	= selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS>>3], sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;	// change the DPL
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS>>3], sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;	// change the DPL

		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p_proc->regs.eip= (u32)p_task->initial_eip;
		p_proc->regs.esp= (u32) p_task_stack;
		p_proc->regs.eflags = eflags;	// IF=1, IOPL=1, bit 2 is always 1.

		p_proc->nr_tty = 0;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += (1<<3);
	}

	proc_table[0].ticks = proc_table[0].priority = 15;
	proc_table[1].ticks = proc_table[1].priority =  5;
	proc_table[2].ticks = proc_table[2].priority =  5;
	proc_table[3].ticks = proc_table[3].priority =  5;

        proc_table[1].nr_tty = 0;
        proc_table[2].nr_tty = 1;
        proc_table[3].nr_tty = 1;

	k_reenter	= 0;
	ticks		= 0;

	p_proc_ready	= proc_table;  //还是从进程1开始
	init_clock();

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
//		printf("<Ticks:%x>", get_ticks());
//		disp_str("A.");
//		disp_int(get_ticks());
		milli_delay(200);
	}
}


void TestB()
{
	while(1){
//		printf("B");
//		disp_str("B.");
//		disp_int(get_ticks());
		milli_delay(200);
	}
}

void TestC()
{
	while(1){
//		printf("C");
//		disp_str("C.");
//		disp_int(get_ticks());
		milli_delay(200);
	}
}

