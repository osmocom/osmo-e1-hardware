# Project config
PROJ=e1-tracer

PROJ_DEPS := no2e1 no2ice40 no2misc no2usb
PROJ_RTL_SRCS := $(addprefix rtl/, \
	misc.v \
	sysmgr.v \
)
PROJ_RTL_SRCS += $(addprefix ../common/rtl/, \
	capcnt.v \
	capcnt_sb_mac16.v \
	dfu_helper.v \
	picorv32.v \
	picorv32_ice40_regs.v \
	soc_base.v \
	soc_bram.v \
	soc_iobuf.v \
	soc_picorv32_bridge.v \
	soc_spram.v \
	wb_arbiter.v \
	wb_dma.v \
	wb_epbuf.v \
)
PROJ_PREREQ = \
	$(BUILD_TMP)/boot.hex \
	$(NULL)
PROJ_TOP_SRC := rtl/top.v
PROJ_TOP_MOD := top

# Target config
BOARD ?= e1-tracer
DEVICE := up5k
PACKAGE := sg48

SEED ?= 19
NEXTPNR_ARGS := --no-promote-globals --pre-pack data/clocks.py --seed $(SEED)

ifeq ($(IGNORE_TIMING),1)
	NEXTPNR_ARGS += --timing-allow-fail
endif

# Include default rules
NO2BUILD_DIR ?= ../build
include $(NO2BUILD_DIR)/project-rules.mk

# Custom rules
../common/fw/boot.hex:
	make -C ../common/fw boot.hex

$(BUILD_TMP)/boot.hex: ../common/fw/boot.hex
	cp $< $@
