#!/bin/bash

set -e
USER_SRC=$1
OUT_NAME="user"
LIB_PATH="."
INCLUDE_PATH="."

if [ -z "$USER_SRC" ]; then
    echo "Usage: ./build_user.sh <file.c>"
    exit 1
fi

OUT_NAME=$(basename "$USER_SRC" .c)
echo "[0] Building libobos..."
gcc -m32 -ffreestanding -nostdlib -c crt0.c -o crt0.o
gcc -m32 -ffreestanding -nostdlib -c ObosSys.c -o ObosSys.o
ar rcs libobos.a crt0.o ObosSys.o

echo "[1] Compiling user program..."
gcc -m32 -ffreestanding -nostdlib -fno-pic -fno-pie -I$INCLUDE_PATH -c $USER_SRC -o $OUT_NAME.o

echo "[2] Linking with libobos..."
ld -m elf_i386 -Ttext 0x40000000 -e _start $OUT_NAME.o -L$LIB_PATH -lobos -o $OUT_NAME.elf

echo "[3] Converting to flat binary..."
objcopy -O binary $OUT_NAME.elf $OUT_NAME.bin

echo "[4] Putting into kernel object..."
ld -m elf_i386 -r -b binary $OUT_NAME.bin -o ${OUT_NAME}_bin.o

USER_DIR="../Disk/User"
mkdir -p $USER_DIR
mv ${OUT_NAME}_bin.o $USER_DIR/

echo "Done!"
echo "Generated:"
echo "  ${OUT_NAME}_bin.o(for kernel linking)"
