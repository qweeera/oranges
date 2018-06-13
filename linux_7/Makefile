#Makefile for boot

#programs , flags, etc

ENTRYPOINT	= 0x30400

ENTRYOFFSET	= 0X400


ASM		= nasm
DASM		= ndisasm
CC		= gcc
LD		= ld 
ASMBFLAGS	= -I boot/include/
ASMKFLAGS	= -I include/ -f elf
CFLAGS		= -I include/ -m32 -c -fno-builtin  
LDFLAGS		= -s -Ttext $(ENTRYPOINT)
DASMFLAGS 	= -u -o $(ENTRYPOINT) -e $(ENTRYPOINT)


#this Program
ORANGESBOOT	= boot/boot.bin boot/loader.bin
ORANGESKERNEL	= kernel.bin
OBJS		= kernel/kernel.o kernel/start.o kernel/main.o \
		  kernel/i8259.o kernel/protect.o lib/klib.o lib/kliba.o\
		  lib/string.o  kernel/proc.o kernel/syscall.o \
		  kernel/keyboard.o kernel/clock.o kernel/tty.o \
		  kernel/console.o kernel/printf.o
	
DASMOUTPUT	= kernel.bin.asm

#all Phony Targets
.PHONY : everything final image clean realclean disasm all buildimg

# Default starting position
everything : $(ORANGESBOOT) $(ORANGESKERNEL)

all : realclean everything

final : all clean

image : final buildimg

clean :
	rm -f $(OBJS)

realclean :
	rm -f $(OBJS) $(ORANGESBOOT) $(ORANGESKERNEL)

disasm :
	$(DASM) $(DASMFLAGS) $(ORANGESKERNEL) > $(DASMOUTPUT)

# We assume that "a.img" exists in current folder
buildimg :
	dd if=boot/boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img /mnt/floppy/
	sudo cp -fv boot/loader.bin /mnt/floppy/
	sudo cp -fv kernel.bin /mnt/floppy
	sudo umount /mnt/floppy

boot/boot.bin : boot/boot.asm boot/include/load.inc boot/include/fat12hdr.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin : boot/loader.asm boot/include/load.inc \
			boot/include/fat12hdr.inc boot/include/pm.inc \
			boot/include/lib.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

$(ORANGESKERNEL) : $(OBJS)
	$(LD) -m elf_i386  $(LDFLAGS) -o $(ORANGESKERNEL) $(OBJS)

kernel/kernel.o : kernel/kernel.asm 
	$(ASM) $(ASMKFLAGS) -o $@ $<


kernel/start.o : kernel/start.c include/type.h include/const.h include/protect.h include/proto.h include/string.h 
	$(CC) $(CFLAGS) -o $@ $<

kernel/i8259.o : kernel/i8259.c include/type.h include/const.h include/protect.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/global.o : kernel/global.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/protect.o : kernel/protect.c 
	$(CC) $(CFLAGS) -fno-stack-protector -o $@ $<


lib/klib.o : lib/klib.c
	$(CC) $(CFLAGS) -fno-stack-protector -o $@ $<


lib/kliba.o : lib/kliba.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/string.o : lib/string.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/main.o : kernel/main.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/proc.o : kernel/proc.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/syscall.o : kernel/syscall.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/keyboard.o : kernel/keyboard.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/clock.o : kernel/clock.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/tty.o : kernel/tty.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/console.o : kernel/console.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/printf.o : kernel/printf.c
	$(CC) $(CFLAGS) -o $@ $<
