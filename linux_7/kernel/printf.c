

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                              printf.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "string.h"
#include "proto.h"
/******************************************************************************************
                        可变参数函数调用原理（其中涉及的数字皆为举例）
===========================================================================================

i = 0x23;
j = 0x78;
char fmt[] = "%x%d";
printf(fmt, i, j);

        push    j
        push    i
        push    fmt
        call    printf
        add     esp, 3 * 4


                ┃        HIGH        ┃                        ┃        HIGH        ┃
                ┃        ...         ┃                        ┃        ...         ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
                ┃                    ┃                 0x32010┃        '\0'        ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
         0x3046C┃        0x78        ┃                 0x3200c┃         d          ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
   arg = 0x30468┃        0x23        ┃                 0x32008┃         %          ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
         0x30464┃      0x32000 ───╂────┐       0x32004┃         x          ┃
                ┣━━━━━━━━━━┫        │              ┣━━━━━━━━━━┫
                ┃                    ┃        └──→ 0x32000┃         %          ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
                ┃        ...         ┃                        ┃        ...         ┃
                ┃        LOW         ┃                        ┃        LOW         ┃

实际上，调用 vsprintf 的情形是这样的：

        vsprintf(buf, 0x32000, 0x30468);

******************************************************************************************/


int vsprintf(char *buf, const char *fmt, va_list args);


/*======================================================================*
                                 printf
 *======================================================================*/
int printf(const char *fmt, ...)
{
	int i;
	static char buf[256];

	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	i = vsprintf(buf, fmt, arg);
	write(buf, i);

	return i;
}


/*
 *  为更好地理解此函数的原理，可参考 printf 的注释部分。
 */

/*======================================================================*
                                vsprintf
 *======================================================================*/
int vsprintf(char *buf, const char *fmt, va_list args)
{
	char*	p;
	static  char	tmp[256];
	va_list	p_next_arg = args;

	for (p=buf;*fmt;fmt++) {
		if (*fmt != '%') {
			*p++ = *fmt;
			continue;
		}

		fmt++;

		switch (*fmt) {
		case 'x':
			itoa(tmp, *((int*)p_next_arg));
			strcpy(p, tmp);
			p_next_arg += 4;
			p += strlen(tmp);
			break;
		case 's':
			break;
		default:
			break;
		}
	}

	return (p - buf);
}