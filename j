sudo qemu-system-i386 \
    -machine pc,accel=tcg \
    -drive file=Disk/obos.img,format=raw \
    -netdev tap,id=net0,ifname=tap0,script=no,downscript=no \
    -device e1000,netdev=net0 \
    -monitor stdio