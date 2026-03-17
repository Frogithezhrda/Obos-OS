rm -f qemu.pcap
qemu-system-i386 -drive file=Disk/obos.img,format=raw \
    -netdev user,id=net0 \
    -device e1000,netdev=net0 \
    -object filter-dump,id=f1,netdev=net0,file=qemu.pcap \
    -monitor stdio