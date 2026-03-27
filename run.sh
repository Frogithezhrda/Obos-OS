qemu-system-i386 -drive file=Disk/obos.img,format=raw \
	-netdev user,id=net0 \
	-device e1000,netdev=net0