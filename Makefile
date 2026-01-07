# output directories
DISK_DIR = Disk
COMPONENTS_DIR = $(DISK_DIR)/Components

# output files
DISK_IMAGE_FILE_PATH = $(DISK_DIR)/obos.img
BOOT_BIN = $(COMPONENTS_DIR)/boot.bin
KERNEL_BIN = $(COMPONENTS_DIR)/kernel.bin
KERNEL_ASM_OBJ = $(COMPONENTS_DIR)/kernel_asm.o
ISR_ASM_OBJ = $(COMPONENTS_DIR)/isr.o 
# source files
ATA_SRC = Bootloader/ata.asm
BOOT_SRC = Bootloader/boot.asm
KERNEL_ASM = Kernel/Base/kernel.asm
ISR_ASM = Kernel/Tables/isr.asm
LINKER_SCRIPT = obosLinker.ld

# automatically find all C files in Kernel directory
KERNEL_C_SOURCES = $(shell find Kernel -name '*.c')
KERNEL_C_OBJECTS = $(patsubst %.c,$(COMPONENTS_DIR)/%.o,$(notdir $(KERNEL_C_SOURCES)))

# compiler flags
CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -nodefaultlibs -Wall -Wextra

# search paths for source files
vpath %.c Kernel/Base Kernel/Drivers Kernel/Tables Kernel/SystemLib

all: $(DISK_IMAGE_FILE_PATH)

$(DISK_IMAGE_FILE_PATH): $(BOOT_BIN) $(KERNEL_BIN) $(ISR_ASM_OBJ)
	@echo "------ Creating/Updating image ------"
	@mkdir -p $(DISK_DIR)
	@if [ ! -f "$(DISK_IMAGE_FILE_PATH)" ]; then \
		qemu-img create -f raw "$(DISK_IMAGE_FILE_PATH)" 1G; \
	fi
	@dd if=$(BOOT_BIN) of=$(DISK_IMAGE_FILE_PATH) bs=512 seek=0 conv=notrunc
	@dd if=$(KERNEL_BIN) of=$(DISK_IMAGE_FILE_PATH) bs=512 seek=1 conv=notrunc

$(BOOT_BIN): $(BOOT_SRC) $(ATA_SRC) 
	@echo "------ Assembling bootloader ------"
	@mkdir -p $(COMPONENTS_DIR)
	nasm -f bin $(BOOT_SRC) -o $(BOOT_BIN)

$(ISR_ASM_OBJ): $(ISR_ASM)
	@echo "------ Assembling ISR handlers ------"
	@mkdir -p $(COMPONENTS_DIR)
	@nasm -f elf32 $(ISR_ASM) -o $(ISR_ASM_OBJ)

$(KERNEL_ASM_OBJ): $(KERNEL_ASM)
	@echo "------ Assembling kernel entry ------"
	@mkdir -p $(COMPONENTS_DIR)
	@nasm -f elf32 $(KERNEL_ASM) -o $(KERNEL_ASM_OBJ)

# generic rule for compiling any C file
$(COMPONENTS_DIR)/%.o: %.c
	@echo "------ Compiling $< ------"
	@mkdir -p $(COMPONENTS_DIR)
	@gcc $(CFLAGS) -c $< -o $@

$(KERNEL_BIN): $(KERNEL_ASM_OBJ) $(KERNEL_C_OBJECTS) $(LINKER_SCRIPT) $(ISR_ASM_OBJ)
	@echo "------ Linking kernel ------"
	@mkdir -p $(COMPONENTS_DIR)
	@ld -m elf_i386 -T $(LINKER_SCRIPT) -o $(KERNEL_BIN) $(KERNEL_ASM_OBJ) $(ISR_ASM_OBJ) $(KERNEL_C_OBJECTS)
clean:
	@echo "------ Cleaning up ------"
	@rm -rf $(COMPONENTS_DIR)

.PHONY: all clean