/*
 * spi.h
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include <stdbool.h>

struct spi_xfer_chunk {
	uint8_t *data;
	unsigned len;
	bool write;
	bool read;
};

#define SPI_CS_FLASH	0
#define SPI_CS_LIU(n)	(4+(n))

void spi_init(void);
void spi_xfer(unsigned cs, const struct spi_xfer_chunk *xfer, unsigned n);

void flash_cmd(uint8_t cmd);
void flash_deep_power_down(void);
void flash_wake_up(void);
void flash_write_enable(void);
void flash_write_disable(void);
void flash_manuf_id(void *manuf);
void flash_unique_id(void *id);
uint8_t flash_read_sr(void);
void flash_write_sr(uint8_t sr);
void flash_read(void *dst, uint32_t addr, unsigned len);
void flash_page_program(const void *src, uint32_t addr, unsigned len);
void flash_sector_erase(uint32_t addr);
