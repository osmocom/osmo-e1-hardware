BOARD := ice1usb
CROSS ?= riscv-none-embed-
CC = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
ICEPROG = iceprog
DFU_UTIL = dfu-util

BOARD_DEFINE=BOARD_$(shell echo $(BOARD) | tr a-z\- A-Z_)
CFLAGS=-Wall -Os -march=rv32i -mabi=ilp32 -ffreestanding -flto -nostartfiles -fomit-frame-pointer -Wl,--gc-section --specs=nano.specs -D$(BOARD_DEFINE) -I. -I../common

NO2USB_FW_VERSION=0
include ../../../gateware/cores/no2usb/fw/fw.mk
CFLAGS += $(INC_no2usb)

LNK=../common/lnk-app.lds

HEADERS_common := $(addprefix ../common/, \
	console.h \
	dma.h \
	led.h \
	mini-printf.h \
	spi.h \
	utils.h \
)

SOURCES_common := $(addprefix ../common/, \
	start.S \
	console.c \
	dma.c \
	led.c \
	mini-printf.c  \
	spi.c \
	utils.c \
)

HEADERS_common += $(HEADERS_no2usb)
SOURCES_common += $(SOURCES_no2usb)

HEADERS_app=\
	usb_str_app.gen.h \
	$(NULL)

SOURCES_app=\
	fw_app.c \
	usb_desc_app.c \
	$(NULL)

HEADERS_app += $(addprefix ../icE1usb/, \
	config.h \
	e1.h \
	misc.h \
)

SOURCES_app += $(addprefix ../icE1usb/, \
	e1.c \
	misc.c \
)

CFLAGS += -I../icE1usb/ -DTEST=1


all: fw_app.bin


fw_app.elf: $(LNK) $(HEADERS_app) $(SOURCES_app) $(HEADERS_common) $(SOURCES_common)
	$(CC) $(CFLAGS) -Wl,-Bstatic,-T,$(LNK),--strip-debug -o $@ $(SOURCES_common) $(SOURCES_app)


%.hex: %.bin
	../common/bin2hex.py $< $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

prog: fw_app.bin
	$(ICEPROG) -o 640k $<

dfuprog: fw_app.bin
	$(DFU_UTIL) -R -a 1 -D $<


clean:
	rm -f *.bin *.hex *.elf *.o *.gen.h

.PHONY: prog dfuprog clean
