sudo qemu-system-i386 \
    -machine pc,accel=tcg,kernel_irqchip=on \
    -drive file=Disk/obos.img,format=raw \
    -netdev tap,id=net0,ifname=tap0,script=no,downscript=no \
    -device rtl8139,netdev=net0 \
    -monitor stdio