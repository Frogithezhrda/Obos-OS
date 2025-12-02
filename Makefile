# output directories
DISK_DIR = Disk
COMPONENTS_DIR = $(DISK_DIR)/Components

# output files
DISK_IMAGE_FILE_PATH = $(DISK_DIR)/obos.img
BOOT_BIN = $(COMPONENTS_DIR)/boot.bin
KERNEL_BIN = $(COMPONENTS_DIR)/kernel.bin
KERNEL_ASM_OBJ = $(COMPONENTS_DIR)/kernel_asm.o
KERNEL_C_OBJ = $(COMPONENTS_DIR)/kernel_c.o
CONSOLE_DRIVER_OBJ = $(COMPONENTS_DIR)/consoleDriver.o
# source files
BOOT_SRC = Bootloader/boot.asm
KERNEL_ASM = Kernel/Base/kernel.asm
KERNEL_C = Kernel/Base/kernel.c
LINKER_SCRIPT = obosLinker.ld
CONSOLE_DRIVER_C = Kernel/Drivers/consoleDriver.c
# compiler flags
CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -nodefaultlibs -Wall -Wextra

all: $(DISK_IMAGE_FILE_PATH)

$(DISK_IMAGE_FILE_PATH): $(BOOT_BIN) $(KERNEL_BIN)
	@echo "------ Creating/Updating image ------"
	@mkdir -p $(DISK_DIR)
	@if [ ! -f "$(DISK_IMAGE_FILE_PATH)" ]; then \
		qemu-img create -f raw "$(DISK_IMAGE_FILE_PATH)" 1G; \
	fi
	@dd if=$(BOOT_BIN) of=$(DISK_IMAGE_FILE_PATH) bs=512 seek=0 conv=notrunc
	@dd if=$(KERNEL_BIN) of=$(DISK_IMAGE_FILE_PATH) bs=512 seek=1 conv=notrunc

$(BOOT_BIN): $(BOOT_SRC)
	@echo "------ Assembling bootloader ------"
	@mkdir -p $(COMPONENTS_DIR)
	@nasm -f bin $(BOOT_SRC) -o $(BOOT_BIN)

$(KERNEL_ASM_OBJ): $(KERNEL_ASM)
	@echo "------ Assembling kernel entry ------"
	@mkdir -p $(COMPONENTS_DIR)
	@nasm -f elf32 $(KERNEL_ASM) -o $(KERNEL_ASM_OBJ)

$(KERNEL_C_OBJ): $(KERNEL_C)
	@echo "------ Compiling C kernel ------"
	@mkdir -p $(COMPONENTS_DIR)
	@gcc $(CFLAGS) -c $(KERNEL_C) -o $(KERNEL_C_OBJ)

$(CONSOLE_DRIVER_OBJ): $(CONSOLE_DRIVER_C)
	@echo "------ Compiling console driver ------"
	@mkdir -p $(COMPONENTS_DIR)
	@gcc $(CFLAGS) -c $(CONSOLE_DRIVER_C) -o $(CONSOLE_DRIVER_OBJ)

$(KERNEL_BIN): $(KERNEL_ASM_OBJ) $(KERNEL_C_OBJ) $(CONSOLE_DRIVER_OBJ) $(LINKER_SCRIPT)
	@echo "------ Linking kernel ------"
	@mkdir -p $(COMPONENTS_DIR)
	@ld -m elf_i386 -T $(LINKER_SCRIPT) -o $(KERNEL_BIN) $(KERNEL_ASM_OBJ) $(KERNEL_C_OBJ) $(CONSOLE_DRIVER_OBJ)



clean:
	@echo "------ Cleaning up ------"
	@rm -rf $(COMPONENTS_DIR)