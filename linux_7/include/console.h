
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_CONSOLE_H_
#define _ORANGES_CONSOLE_H_
#include "type.h"

#define TTY_IN_BYTES    256     /* tty input queue size */

#define SCR_UP	1	/* scroll forward */
#define SCR_DN	-1	/* scroll backward */

#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH		80

/* CONSOLE */
typedef struct s_console
{
	unsigned int	current_start_addr;	/* 当前显示到了什么位置	  */
	unsigned int	original_addr;		/* 当前控制台对应显存位置 */
	unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
	unsigned int	cursor;			/* 当前光标位置 */
}CONSOLE;

/* TTY */
typedef struct s_tty
{
        u32     in_buf[TTY_IN_BYTES];   /* TTY 输入缓冲区 */
        u32*    p_inbuf_head;           /* 指向缓冲区中下一个空闲位置 */
        u32*    p_inbuf_tail;           /* 指向键盘任务应处理的键值 */
        int     inbuf_count;            /* 缓冲区中已经填充了多少 */

        struct s_console *      p_console;
}TTY;


/*console.c*/
#define DEFAULT_CHAR_COLOR	0x07	/* 0000 0111 黑底白字 */
extern int nr_current_console;
extern int is_current_console(CONSOLE* p_con);
extern void out_char(CONSOLE* p_con, char ch);
extern void init_screen(TTY* p_tty);
extern void select_console(int nr_console);
extern void scroll_screen(CONSOLE* p_con, int direction);

/*tty.c*/
extern TTY              tty_table[];
extern CONSOLE          console_table[];

extern void task_tty(void);
extern void in_process(TTY* p_tty, u32 key);

#endif /* _ORANGES_CONSOLE_H_ */
