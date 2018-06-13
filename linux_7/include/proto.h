
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef __PROTO_H
#define __PROTO_H
#include "type.h"
typedef  struct s_task{
	task_f initial_eip;
	int	stacksize;
	char	name[32];
}TASK;


/* kliba.asm */
extern  void	out_byte(u16 port, u8 value);
extern  u8	in_byte(u16 port);
extern  void	disp_str(char * info);
extern  void	disp_color_str(char * info, int color);
extern  void 	enable_irq(int irq);
extern	void 	disable_irq(int irq);
extern  void 	disable_int();
extern	void	enable_int();


/*klib.c*/
extern  char * itoa(char * str, int num);
extern  void disp_int(int input);
extern  char* strcpy(char *dst, const char *src);

/* klib.c */
extern void	delay(int time);

/* printf.c */
extern int printf(const char *fmt, ...);

/* kernel.asm */
void restart();
extern void sys_call();

/* main.c */
void TestA();
void TestB();
void clock_handler(int irq);
extern int ticks;

/* syscall.asm 用户级*/
extern int get_ticks();
extern void write(char* buf, int len);
/* clock.c */
extern void init_clock(void);
extern void clock_handler(int irq);


#endif
