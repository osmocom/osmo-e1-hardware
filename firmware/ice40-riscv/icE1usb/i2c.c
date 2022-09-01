/*
 * i2c.c
 *
 * Copyright (C) 2021-2022  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>
#include <stdbool.h>

#include "console.h"

#include "config.h"


struct i2c {
	uint32_t csr;
} __attribute__((packed,aligned(4)));

#define I2C_CMD_START	(0 << 12)
#define I2C_CMD_STOP	(1 << 12)
#define I2C_CMD_WRITE	(2 << 12)
#define I2C_CMD_READ	(3 << 12)

#define I2C_GET_RESP	(1 << 15)
#define I2C_ACK		(0 <<  8)	/* ack bit value = 0 means ACK */
#define I2C_NAK		(1 <<  8)	/* ack bit value = 1 means NAK */

#define I2C_VALID	(1 << 31)


static volatile struct i2c * const i2c_regs = (void *)(I2C_BASE);


static inline uint32_t
_i2c_wait(void)
{
	uint32_t v;

	do {
		v = i2c_regs->csr;
	} while (!(v & I2C_VALID));

	return v & 0x1ff;
}

bool
i2c_ready(void)
{
	return i2c_regs->csr & (1 << 31);
}

void
i2c_start(void)
{
	i2c_regs->csr = I2C_CMD_START;
}

void
i2c_stop(void)
{
	i2c_regs->csr = I2C_CMD_STOP;
}

bool
i2c_write(uint8_t data)
{
	i2c_regs->csr = I2C_CMD_WRITE | data;
	return (_i2c_wait() & (I2C_ACK | I2C_NAK)) == I2C_ACK;
}

uint8_t
i2c_read(bool ack)
{
	i2c_regs->csr = I2C_CMD_READ | I2C_GET_RESP | (ack ? I2C_ACK : I2C_NAK);
	return _i2c_wait() & 0xff;
}


bool
i2c_write_reg(uint8_t dev, uint8_t reg, uint8_t val)
{
	bool rv = true;
	i2c_start();
	rv = rv && i2c_write(dev);
	rv = rv && i2c_write(reg);
	rv = rv && i2c_write(val);
	i2c_stop();
	return rv;
}

bool
i2c_read_reg(uint8_t dev, uint8_t reg, uint8_t *val)
{
	bool rv = true;
	i2c_start();
	rv = rv && i2c_write(dev);
	rv = rv && i2c_write(reg);
	if (rv)
		i2c_start();
	rv = rv && i2c_write(dev|1);
	*val = rv ? i2c_read(false) : 0x00; // NAK
	i2c_stop();
	return rv;
}


bool
i2c_probe(uint8_t dev)
{
	bool rv;
	i2c_start();
	rv = i2c_write(dev);
	i2c_stop();
	return rv;
}

void
i2c_scan(void)
{
	for (uint8_t addr=0; addr<128; addr++) {
		if (i2c_probe(addr << 1))
			printf("I2C @ %08x\n", addr << 1);
	}
}
