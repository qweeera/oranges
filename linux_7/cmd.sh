
nasm boot.asm -o boot.bin
nasm loader.asm -o loader.bin


nasm -f elf -o kernel.o kernel.asm
nasm -f elf -o string.o string.asm
nasm -f elf -o kliba.o kliba.asm
gcc -m32  -c -fno-builtin -o start.o start.c


ld -m elf_i386 -s -Ttext 0x30400 -o  kernel.bin  kernel.o string.o start.o kliba.o


dd if=boot.bin of=a.img bs=512 count=1 conv=notrunc

sudo mount -o loop a.img /mnt/floppy/

sudo cp loader.bin /mnt/floppy/ -v
sudo cp	kernel.bin /mnt/floppy/ -v
sudo umount /mnt/floppy/

#gcc -m32 -c  -o bar.o bar.c
