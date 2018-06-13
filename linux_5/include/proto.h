
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef __PROTO_H
#define __PROTO_H

#include "type.h"

/* klib.asm */
extern  void	out_byte(u16 port, u8 value);
extern  u8	in_byte(u16 port);
extern  void	disp_str(char * info);
extern  void	disp_color_str(char * info, int color);

/*klib.c*/
extern  void disp_int(int input);

#endif
