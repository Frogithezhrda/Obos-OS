#!/bin/bash
qemu-system-i386 \
    -machine pc,accel=tcg,kernel_irqchip=on \
    -drive file=Disk/obos.img,format=raw \
    -netdev user,id=net0 \
    -device rtl8139,netdev=net0 \
    -object filter-dump,id=f1,netdev=net0,file=net.pcap \
    -audiodev pa,id=audio0,server=/run/user/1000/pulse/native \
    -device sb16,audiodev=audio0