#include "const.h"
#include "proto.h"
#include "i8259.h"
#include "keyboard.h"
#include "keymap.h"
static	KB_INPUT	kb_in;

static int	code_with_E0 = 0;
static int	shift_l;
static int 	shift_r;
static int	alt_l;
static int	alt_r;
static int	ctrl_l;
static int 	ctrl_r;
static int	caps_lock;
static int 	num_lock;
static int	scroll_lock;
static int 	column;

static int	caps_lock;	/* Caps Lock	 */
static int	num_lock;	/* Num Lock	 */
static int	scroll_lock;	/* Scroll Lock	 */

static u8	get_byte_from_kbuf();
static void    set_leds();
static void    kb_wait();
static void    kb_ack();



//中断目前的功能 将到来的一个字节存储到缓冲区
void keyboard_handler(int irq)
{
//	u8 scan_code = 	in_byte(0x60);
//	disp_int(scan_code);
	u8 scan_code = 	in_byte(KB_DATA);

	if(kb_in.count < KB_IN_BYTES){
		*(kb_in.p_head) = scan_code;
		kb_in.p_head++;
		if(kb_in.p_head == kb_in.buf + KB_IN_BYTES){
			kb_in.p_head = kb_in.buf;
		}
		kb_in.count++;
	}
}
		


void init_keyboard(void)
{
	//缓冲区初始化
	kb_in.count = 0;
	kb_in.p_head = kb_in.p_tail = kb_in.buf;

	shift_l	= shift_r = 0;
	alt_l	= alt_r   = 0;
	ctrl_l	= ctrl_r  = 0;

	caps_lock   = 0;
	num_lock    = 1;
	scroll_lock = 0;

	set_leds();


	put_irq_handler(KEYBOARD_IRQ, keyboard_handler);  /*设定键盘中断处理程序*/
	enable_irq(KEYBOARD_IRQ);
}

void keyboard_read(TTY* p_tty)
{
	u8 scan_code;
	static char output[2];
	int make;
	u32 key = 0;    /*用一个整型来表示一个键*/
	u32 * keyrow;   /*指向keymap 某一行*/
/*	memset(output, 0, 2);  */

	if(kb_in.count > 0){
		code_with_E0 = 0;
		scan_code = get_byte_from_kbuf();

		/*解析扫描码*/
		if(scan_code == 0xE1){
			int i;
			static u8 pausebrk_scode[] = {0xE1, 0x1D, 0x45,
						      0xE1, 0x9D, 0xC5};
			int is_pausebreak = 1;
			for(i=1; i<6; i++){
				if(get_byte_from_kbuf() != pausebrk_scode[i]){
					is_pausebreak = 0;
					break;
				}
			}
			if(is_pausebreak){
				key = PAUSEBREAK;
			}
		}
		else if(scan_code == 0xE0){
			scan_code = get_byte_from_kbuf();
			
			/*PrintScreen被按下*/
			if(scan_code == 0x2A){
				if(get_byte_from_kbuf() == 0xE0){
					if(get_byte_from_kbuf() == 0x37){
						key = PRINTSCREEN;
						make = 1;
					}
				}
			}
			/*PrintScreen被释放*/
			if(scan_code == 0xB7){
				if(get_byte_from_kbuf() == 0xE0){
					if(get_byte_from_kbuf() == 0xAA){
						key = PRINTSCREEN;
						make = 0;
					}
				}
			}
			/*不是printscreen,此时scan_code为oxe0紧跟的那个值*/
			if(key == 0){
				code_with_E0 = 1;
			}
		}
		if((key != PAUSEBREAK) && (key != PRINTSCREEN)){
			/*判断是make码 还是break码*/
			make = (scan_code & FLAG_BREAK ? FALSE : TRUE);
			/*定位到keymap中的行*/
			keyrow = &keymap[(scan_code & 0x7f) * MAP_COLS];
			
			/*确定列*/
			column = 0;  
			if(shift_l || shift_r){
				column = 1;
			}
			if(code_with_E0){
				column = 2;
				code_with_E0 = 0;
			}
			/*确定字符*/
			key = keyrow[column];
			
			switch(key){
			case SHIFT_L:
				shift_l = make;
				key = 0;
				break;
			case SHIFT_R:
				shift_r = make;
				key = 0;
				break;
			case CTRL_L:
				ctrl_l = make;
				key = 0;
				break;
			case CTRL_R:
				ctrl_r = make;
				key = 0;
				break;
			case ALT_L:
				alt_l = make;
				key = 0;
				break;
			case ALT_R:
				alt_r = make;
				key = 0;
				break;
			default:
				break;
			}
			if(make){   /* 忽略break code */
				key |= shift_l ? FLAG_SHIFT_L : 0;
				key |= shift_r ? FLAG_SHIFT_R : 0;
				key |= ctrl_l ? FLAG_CTRL_L   : 0;
				key |= ctrl_r ? FLAG_CTRL_R   : 0;
				key |= alt_l ? FLAG_ALT_L     : 0;
				key |= alt_r ? FLAG_ALT_R     : 0;

				in_process(p_tty, key);
			
			}
		} 
	}
}


static u8 get_byte_from_kbuf()
{
	u8 scan_code;
	while(kb_in.count <= 0){}
	disable_int();
	scan_code = *(kb_in.p_tail);
	kb_in.p_tail++;
	if(kb_in.p_tail == kb_in.buf + KB_IN_BYTES){
		kb_in.p_tail = kb_in.buf;
	}
	kb_in.count--;
	enable_int();

	return scan_code;
}




/*======================================================================*
				 kb_wait
 *======================================================================*/
static void kb_wait()	/* 等待 8042 的输入缓冲区空 */
{
	u8 kb_stat;

	do {
		kb_stat = in_byte(KB_CMD);
	} while (kb_stat & 0x02);
}


/*======================================================================*
				 kb_ack
 *======================================================================*/
static void kb_ack()
{
	u8 kb_read;

	do {
		kb_read = in_byte(KB_DATA);
	} while (kb_read =! KB_ACK);
}

/*======================================================================*
				 set_leds
 *======================================================================*/
static void set_leds()
{
	u8 leds = (caps_lock << 2) | (num_lock << 1) | scroll_lock;

	kb_wait();
	out_byte(KB_DATA, LED_CODE);
	kb_ack();

	kb_wait();
	out_byte(KB_DATA, leds);
	kb_ack();
}

