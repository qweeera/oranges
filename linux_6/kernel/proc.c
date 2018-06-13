
#include "proto.h"
#include "protect.h"
int sys_get_ticks(void)
{
//	disp_str("z");	这句话必须注释掉因为显示也是系统调用   
	return ticks;
}

void schedule(void)
{
	PROCESS* p;
	int  greatest_ticks = 0;

	while(!greatest_ticks)
	{
		for(p = proc_table; p < proc_table + NR_TASKS; p++){
			if(p->ticks > greatest_ticks){
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if(!greatest_ticks){
			for(p = proc_table; p<proc_table+ NR_TASKS; p++){
				p->ticks = p->priority;
			}
		}
	}
}
