CROSS ?= riscv-none-elf-

CC := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy

CFLAGS=-Wall -Os -march=rv32i -mabi=ilp32 -ffreestanding -flto -nostartfiles -fomit-frame-pointer -Wl,--gc-section --specs=nano.specs


all: boot.hex


boot.elf: lnk-boot.lds boot.S
	$(CC) $(CFLAGS) -Wl,-Bstatic,-T,lnk-boot.lds,--strip-debug -DAPP_FLASH_ADDR=0x000a0000 -o $@ boot.S

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.hex: %.bin
	hexdump boot.bin -ve '1/4 "%08x\n"' $< > $@

clean:
	rm -f *.bin *.hex *.elf *.o

.PHONY: clean
