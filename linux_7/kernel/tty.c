#include "proto.h"
#include "keyboard.h"
#include "const.h"
#include "console.h"
#include "protect.h"
TTY		tty_table[NR_CONSOLES];
CONSOLE		console_table[NR_CONSOLES];


#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

static void tty_do_read(TTY* p_tty);
static void tty_do_write(TTY* p_tty);
static void put_key(TTY* p_tty, u32 key);

/*每个终端对应一个缓冲区  首先对其进行初始化 指定对应的console*/
static void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);
		
}



void task_tty()
{
	TTY* p_tty;
	init_keyboard();
	for(p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++){
		init_tty(p_tty);   //初始化每一个终端
	}
	select_console(0);
	while(1){
		for(p_tty = TTY_FIRST; p_tty<TTY_END; p_tty++){
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}


void in_process(TTY* p_tty, u32 key)
{
	static char output[2] = {'\0', '\0'};
	/*存一个字节到当前p_tty的缓冲区*/
	if(!(key & FLAG_EXT)){
		put_key(p_tty, key);
	}
	else{
		int raw_code = key & MASK_RAW;
                switch(raw_code) {
		case ENTER:
			put_key(p_tty, '\n');
			break;
		case BACKSPACE:
			put_key(p_tty, '\b');
			break;
                case UP:
                        if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) 
                        	scroll_screen(p_tty->p_console, SCR_DN);
                        break;
                case DOWN:
                        if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) 
                        	scroll_screen(p_tty->p_console, SCR_UP);
                        break;
		case F1:
		case F2:
		case F3:
		case F4:
		case F5:
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			/* alt + F1~F12 */	
			if((key & FLAG_ALT_L) || (key & FLAG_ALT_R)){
				select_console(raw_code - F1);
			}
			break;
                default:
                        break;
                }
	}
}

static void tty_do_read(TTY* p_tty)
{
        if(is_current_console(p_tty->p_console)){
                keyboard_read(p_tty);
        }
}

static void tty_do_write(TTY* p_tty)
{
	if(p_tty->inbuf_count){
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if(p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES){
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;
		out_char(p_tty->p_console, ch);	
	}
}

static void put_key(TTY* p_tty, u32 key)
{

	if(p_tty->inbuf_count < TTY_IN_BYTES){
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if(p_tty->p_inbuf_head == p_tty->in_buf+TTY_IN_BYTES){
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}



/*======================================================================*
                              tty_write
*======================================================================*/
void tty_write(TTY* p_tty, char* buf, int len)
{
        char* p = buf;
        int i = len;

        while (i) {
                out_char(p_tty->p_console, *p++);
                i--;
        }
}

/*======================================================================*
                              sys_write
*======================================================================*/
int sys_write(char* buf, int len, PROCESS* p_proc)
{
        tty_write(&tty_table[p_proc->nr_tty], buf, len);
        return 0;
}

