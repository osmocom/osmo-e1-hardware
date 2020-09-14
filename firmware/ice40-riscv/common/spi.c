/*
 * spi.c
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "spi.h"


struct spi {
	uint32_t _rsvd0[6];
	uint32_t irq;		/* 0110 - SPIIRQ   - Interrupt Status Register  */
	uint32_t irqen;		/* 0111 - SPIIRQEN - Interrupt Control Register */
	uint32_t cr0;		/* 1000 - CR0      - Control Register 0 */
	uint32_t cr1;		/* 1001 - CR1      - Control Register 1 */
	uint32_t cr2;		/* 1010 - CR2      - Control Register 2 */
	uint32_t br;		/* 1011 - BR       - Baud Rate Register */
	uint32_t sr;		/* 1100 - SR       - Status Register    */
	uint32_t txdr;		/* 1101 - TXDR     - Transmit Data Register */
	uint32_t rxdr;		/* 1110 - RXDR     - Receive Data Register  */
	uint32_t csr;		/* 1111 - CSR      - Chip Select Register   */
} __attribute__((packed,aligned(4)));

#define SPI_CR0_TIDLE(xcnt)	(((xcnt) & 3) << 6)
#define SPI_CR0_TTRAIL(xcnt)	(((xcnt) & 7) << 3)
#define SPI_CR0_TLEAD(xcnt)	(((xcnt) & 7) << 0)

#define SPI_CR1_ENABLE		(1 << 7)
#define SPI_CR1_WKUPEN_USER	(1 << 6)
#define SPI_CR1_TXEDGE		(1 << 4)

#define SPI_CR2_MASTER		(1 << 7)
#define SPI_CR2_MCSH		(1 << 6)
#define SPI_CR2_SDBRE		(1 << 5)
#define SPI_CR2_CPOL		(1 << 2)
#define SPI_CR2_CPHA		(1 << 1)
#define SPI_CR2_LSBF		(1 << 0)

#define SPI_SR_TIP		(1 << 7)
#define SPI_SR_BUSY		(1 << 6)
#define SPI_SR_TRDY		(1 << 4)
#define SPI_SR_RRDY		(1 << 3)
#define SPI_SR_TOE		(1 << 2)
#define SPI_SR_ROE		(1 << 1)
#define SPI_SR_MDF		(1 << 0)


static volatile struct spi * const spi_regs[] = {
	(void*)(SPI_FLASH_BASE),
#ifdef BOARD_E1_TRACER
	(void*)(SPI_LIU_BASE),
#endif
};


void
spi_init(void)
{
	/* Channel 0: Flash */
	spi_regs[0]->cr0 = SPI_CR0_TIDLE(3) |
	                   SPI_CR0_TTRAIL(7) |
	                   SPI_CR0_TLEAD(7);

	spi_regs[0]->cr1 = SPI_CR1_ENABLE;
	spi_regs[0]->cr2 = SPI_CR2_MASTER | SPI_CR2_MCSH;
	spi_regs[0]->br  = 3;
	spi_regs[0]->csr = 0xf;

#ifdef BOARD_E1_TRACER
	/* Channel 1: LIU */
	spi_regs[1]->cr0 = SPI_CR0_TIDLE(3) |
	                   SPI_CR0_TTRAIL(7) |
	                   SPI_CR0_TLEAD(7);
	spi_regs[1]->cr1 = SPI_CR1_ENABLE | SPI_CR1_TXEDGE;
	spi_regs[1]->cr2 = SPI_CR2_MASTER | SPI_CR2_LSBF | SPI_CR2_MCSH | SPI_CR2_CPHA;
	spi_regs[1]->br  = 3;
	spi_regs[1]->csr = 0xf;
#endif
}

void
spi_xfer(unsigned cs, const struct spi_xfer_chunk *xfer, unsigned n)
{
	unsigned chan = (cs >> 2);
	cs &= 3;

	/* Setup CS */
	//spi_regs[chan]->cr2 |= SPI_CR2_MCSH;
	spi_regs[chan]->csr = 0xf ^ (1 << cs);

	/* Run the chunks */
	while (n--) {
		for (int i=0; i<xfer->len; i++)
		{
			spi_regs[chan]->txdr = xfer->write ? xfer->data[i] : 0x00;
			while (!(spi_regs[chan]->sr & SPI_SR_RRDY));
			if (xfer->read)
				xfer->data[i] = spi_regs[chan]->rxdr;
		}
		xfer++;
	}

	/* Clear CS */
	//spi_regs[chan]->cr2 &= ~SPI_CR2_MCSH;
	spi_regs[chan]->csr = 0xf;
}


#define FLASH_CMD_DEEP_POWER_DOWN	0xb9
#define FLASH_CMD_WAKE_UP		0xab
#define FLASH_CMD_WRITE_ENABLE		0x06
#define FLASH_CMD_WRITE_ENABLE_VOLATILE	0x50
#define FLASH_CMD_WRITE_DISABLE		0x04

#define FLASH_CMD_READ_MANUF_ID		0x9f
#define FLASH_CMD_READ_UNIQUE_ID	0x4b

#define FLASH_CMD_READ_SR1		0x05
#define FLASH_CMD_WRITE_SR1		0x01

#define FLASH_CMD_READ_DATA		0x03
#define FLASH_CMD_PAGE_PROGRAM		0x02
#define FLASH_CMD_CHIP_ERASE		0x60
#define FLASH_CMD_SECTOR_ERASE		0x20

void
flash_cmd(uint8_t cmd)
{
	struct spi_xfer_chunk xfer[1] = {
		{ .data = (void*)&cmd, .len = 1, .read = false, .write = true,  },
	};
	spi_xfer(SPI_CS_FLASH, xfer, 1);
}

void
flash_deep_power_down(void)
{
	flash_cmd(FLASH_CMD_DEEP_POWER_DOWN);
}

void
flash_wake_up(void)
{
	flash_cmd(FLASH_CMD_WAKE_UP);
}

void
flash_write_enable(void)
{
	flash_cmd(FLASH_CMD_WRITE_ENABLE);
}

void
flash_write_enable_volatile(void)
{
	flash_cmd(FLASH_CMD_WRITE_ENABLE_VOLATILE);
}

void
flash_write_disable(void)
{
	flash_cmd(FLASH_CMD_WRITE_DISABLE);
}

void
flash_manuf_id(void *manuf)
{
	uint8_t cmd = FLASH_CMD_READ_MANUF_ID;
	struct spi_xfer_chunk xfer[2] = {
		{ .data = (void*)&cmd,  .len = 1, .read = false, .write = true,  },
		{ .data = (void*)manuf, .len = 3, .read = true,  .write = false, },
	};
	spi_xfer(SPI_CS_FLASH, xfer, 2);
}

void
flash_unique_id(void *id)
{
	uint8_t cmd = FLASH_CMD_READ_UNIQUE_ID;
	struct spi_xfer_chunk xfer[3] = {
		{ .data = (void*)&cmd, .len = 1, .read = false, .write = true,  },
		{ .data = (void*)0,    .len = 4, .read = false, .write = false, },
		{ .data = (void*)id,   .len = 8, .read = true,  .write = false, },
	};
	spi_xfer(SPI_CS_FLASH, xfer, 3);
}

uint8_t
flash_read_sr(void)
{
	uint8_t cmd = FLASH_CMD_READ_SR1;
	uint8_t rv;
	struct spi_xfer_chunk xfer[2] = {
		{ .data = (void*)&cmd, .len = 1, .read = false, .write = true,  },
		{ .data = (void*)&rv,  .len = 1, .read = true,  .write = false, },
	};
	spi_xfer(SPI_CS_FLASH, xfer, 2);
	return rv;
}

void
flash_write_sr(uint8_t sr)
{
	uint8_t cmd[2] = { FLASH_CMD_WRITE_SR1, sr };
	struct spi_xfer_chunk xfer[1] = {
		{ .data = (void*)cmd, .len = 2, .read = false, .write = true,  },
	};
	spi_xfer(SPI_CS_FLASH, xfer, 1);
}

void
flash_read(void *dst, uint32_t addr, unsigned len)
{
	uint8_t cmd[4] = { FLASH_CMD_READ_DATA, ((addr >> 16) & 0xff), ((addr >> 8) & 0xff), (addr & 0xff)  };
	struct spi_xfer_chunk xfer[2] = {
		{ .data = (void*)cmd, .len = 4,   .read = false, .write = true,  },
		{ .data = (void*)dst, .len = len, .read = true,  .write = false, },
	};
	spi_xfer(SPI_CS_FLASH, xfer, 2);
}

void
flash_page_program(void *src, uint32_t addr, unsigned len)
{
	uint8_t cmd[4] = { FLASH_CMD_PAGE_PROGRAM, ((addr >> 16) & 0xff), ((addr >> 8) & 0xff), (addr & 0xff)  };
	struct spi_xfer_chunk xfer[2] = {
		{ .data = (void*)cmd, .len = 4,   .read = false, .write = true, },
		{ .data = (void*)src, .len = len, .read = false, .write = true, },
	};
	spi_xfer(SPI_CS_FLASH, xfer, 2);
}

void
flash_sector_erase(uint32_t addr)
{
	uint8_t cmd[4] = { FLASH_CMD_SECTOR_ERASE, ((addr >> 16) & 0xff), ((addr >> 8) & 0xff), (addr & 0xff)  };
	struct spi_xfer_chunk xfer[1] = {
		{ .data = (void*)cmd, .len = 4,   .read = false, .write = true,  },
	};
	spi_xfer(SPI_CS_FLASH, xfer, 1);
}
