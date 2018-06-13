#ifndef __I8259_h
#define __I8259_h

#include "const.h"
#include "proto.h"
#include "protect.h"
extern void init_8259A(void);
extern void purious_irq(int irq);

#endif
