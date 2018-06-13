#ifndef __PROC_H
#define __PROC_H
#include "type.h"

#define NR_PROCS		3
#define NR_TASKS                1       
/* stacks of tasks */
#define STACK_SIZE_TESTA        0x8000
#define STACK_SIZE_TESTB        0x8000
#define STACK_SIZE_TESTC        0x8000

#define STACK_SIZE_TTY          0x8000
#define STACK_SIZE_TOTAL        (STACK_SIZE_TESTA + STACK_SIZE_TESTB + STACK_SIZE_TESTC + STACK_SIZE_TTY          )

/* 每个任务有一个单独的 LDT, 每个 LDT 中的描述符个数: */
#define LDT_SIZE                2

/* 存储段描述符/系统段描述符 */
typedef struct s_descriptor             /* 共 8 个字节 */
{
        u16     limit_low;              /* Limit */
        u16     base_low;               /* Base */
        u8      base_mid;               /* Base */
        u8      attr1;                  /* P(1) DPL(2) DT(1) TYPE(4) */
        u8      limit_high_attr2;       /* G(1) D(1) 0(1) AVL(1) LimitHigh(4) */
        u8      base_high;              /* Base */
}DESCRIPTOR;



typedef struct s_stackframe {
        u32     gs;             /* \                                    */
        u32     fs;             /* |                                    */
        u32     es;             /* |                                    */
        u32     ds;             /* |                                    */
        u32     edi;            /* |                                    */
        u32     esi;            /* | pushed by save()                   */
        u32     ebp;            /* |                                    */
        u32     kernel_esp;     /* <- 'popad' will ignore it            */
        u32     ebx;            /* |                                    */
        u32     edx;            /* |                                    */
        u32     ecx;            /* |                                    */
        u32     eax;            /* /                                    */
        u32     retaddr;        /* return addr for kernel.asm::save()   */
        u32     eip;            /* \                                    */
        u32     cs;             /* |                                    */
        u32     eflags;         /* | pushed by CPU during interrupt     */
        u32     esp;            /* |                                    */
        u32     ss;             /* /                                    */
}STACK_FRAME;




typedef struct s_proc {
        STACK_FRAME regs;          /* process registers saved in stack frame */

        u16 ldt_sel;               /* gdt selector giving ldt base and limit */
        DESCRIPTOR ldts[LDT_SIZE]; /* local descriptors for code and data */

        int ticks;
        int priority;

        u32 pid;                   /* process id passed in from MM */
        char p_name[16];           /* name of the process */
        int nr_tty;

}PROCESS;



/* proc.c */
extern int sys_get_ticks(void);
extern void schedule(void);

#endif
