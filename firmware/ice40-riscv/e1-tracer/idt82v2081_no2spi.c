/* (C) 2019 by Harald Welte <laforge@osmocom.org>
 * All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include "idt82v2081.h"
#include "spi.h"

/* Adaption layer between idt82 driver and no2fpga SPI */

/* backend function for core idt82 driver */
int idt82_reg_read(struct idt82 *idt, uint8_t reg)
{
	uint8_t cmd = reg | 0x20;
	uint8_t rv;
	struct spi_xfer_chunk xfer[2] = {
		{ .data = (void *)&cmd, .len = 1, .read = false, .write = true,  },
		{ .data = (void *)&rv,  .len = 1, .read = true,  .write = false, },
	};
	spi_xfer(SPI_CS_LIU(idt->cs), xfer, 2);
	return rv;
}

/* backend function for core idt82 driver */
int idt82_reg_write(struct idt82 *idt, uint8_t reg, uint8_t val)
{
	uint8_t cmd[2] = { reg, val };
	struct spi_xfer_chunk xfer[2] = {
		{ .data = (void *)cmd, .len = 2, .read = false, .write = true,  },
	};
	spi_xfer(SPI_CS_LIU(idt->cs), xfer, 1);
	return 0;
}
