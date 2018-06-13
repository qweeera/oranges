cd /home/lhm/Desktop/linux_4
pwd
#nasm pmtest9_2.asm -o pmtest9f.com
#sudo mount -o loop pm.img /mnt/floppy/
#sudo cp pmtest9f.com /mnt/floppy/
#sudo umount /mnt/floppy/

nasm boot.asm -o boot.bin
nasm loader.asm -o loader.bin

dd if=boot.bin of=a.img bs=512 count=1  conv=notrunc


sudo mount -o loop a.img /mnt/floppy/ 
sudo cp loader.bin /mnt/floppy/ -v
sudo umount /mnt/floppy/

#bochs



