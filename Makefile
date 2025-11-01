

all:
	# temporary for compiling
	nasm -f bin Bootloader/boot.asm -o Bootloader/boot.bin
	dd if=Bootloader/boot.bin of=Disk/obos.img bs=512 seek=1 conv=notrunc
