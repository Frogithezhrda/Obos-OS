qemu-system-i386 \
    -machine pc,accel=tcg,kernel_irqchip=on \
    -drive file=Disk/obos.img,format=raw \
    -netdev user,id=net0,host=10.0.2.2,net=10.0.2.0/24,dns=10.0.2.3,domainname=local \
    -device rtl8139,netdev=net0 \
    -object filter-dump,id=f1,netdev=net0,file=net.pcap \
    -audiodev pa,id=audio0,server=/run/user/1000/pulse/native \
    -device sb16,audiodev=audio0 \
    -vga std