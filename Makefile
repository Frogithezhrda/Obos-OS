DISK_IMAGE_FILE_PATH = Disk/obos.img
BOOT_BIN = Bootloader/boot.bin
KERNEL_BIN = Kernel/Base/kernel.bin

BOOT_SRC = Bootloader/boot.asm
KERNEL_SRC = Kernel/Base/kernel.asm
PROT_BIN = Bootloader/protected.bin
PROT_SRC = Bootloader/protectedMode.asm

all: $(DISK_IMAGE_FILE_PATH)

$(DISK_IMAGE_FILE_PATH): $(BOOT_BIN) $(KERNEL_BIN)
	@echo "------ Creating/Updating image ------"
	@mkdir -p Disk
	
	@if [ ! -f "$(DISK_IMAGE_FILE_PATH)" ]; then \
		echo "Creating disk image..."; \
		qemu-img create -f raw "$(DISK_IMAGE_FILE_PATH)" 1G; \
	fi

	@dd if=$(BOOT_BIN) of=$(DISK_IMAGE_FILE_PATH) bs=512 seek=0 conv=notrunc
	@dd if=$(KERNEL_BIN) of=$(DISK_IMAGE_FILE_PATH) bs=512 seek=1 conv=notrunc

$(BOOT_BIN): $(BOOT_SRC)
	@echo "------ Assembling bootloader ------"
	@nasm -f bin $(BOOT_SRC) -o $(BOOT_BIN)

$(KERNEL_BIN): $(KERNEL_SRC)
	@echo "------ Assembling kernel ------"
	@nasm -f bin $(KERNEL_SRC) -o $(KERNEL_BIN)

clean:
	@echo "------ Cleaning up ------"
	@rm -f $(BOOT_BIN)
	@rm -f $(KERNEL_BIN)
