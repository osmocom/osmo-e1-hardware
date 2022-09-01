/*
 * rs422.c
 *
 * Copyright (C) 2022  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>
#include <stdbool.h>

#include "console.h"
#include "i2c.h"
#include "misc.h"

#include "config.h"


/* Local side buffer control */
#define BUF_TCA9534_ADDR	0x42
#define BUF_TXD_RXEN_n		(1 << 0)
#define BUF_TXD_TXEN_n		(1 << 1)
#define BUF_PPS_RXEN_n		(1 << 4)
#define BUF_PPS_TXEN_n		(1 << 5)
#define BUF_RXD_RXEN_n		(1 << 6)
#define BUF_RXD_TXEN_n		(1 << 7)

/* Isolated side line driver control */
#define LDRV_TCA9534_ADDR	0x40
#define LDRV_TXD_RE_n		(1 << 0)
#define LDRV_TXD_DE		(1 << 1)
#define LDRV_RXD_RE_n		(1 << 2)
#define LDRV_RXD_DE		(1 << 3)
#define LDRV_PPS_DE		(1 << 4)
#define LDRV_PPS_RE_n		(1 << 5)


static void
tca9534_set_out(uint8_t dev, uint8_t data)
{
	/* Check device */
	if (!i2c_probe(dev)) {
		printf("[1] Unable to configure TCA9534 at address %02x\n", dev);
		return;
	}

	/* Output values */
	i2c_write_reg(dev, 1, data);

	/* No inversion */
	i2c_write_reg(dev, 2, 0x00);

	/* Al pins as output to avoid floating pins */
	i2c_write_reg(dev, 3, 0x00);
}


void
rs422_init(void)
{
	/* Reset GPS receiver to free I2C bus */
	gpio_out(3, false);
	gpio_dir(3, true);

	/* Configure:
	 *  - TXD pair: Receive
	 *  - RXD pair: Transmit
	 *  - PPS pair: Receive
	 */
	tca9534_set_out(BUF_TCA9534_ADDR,
		BUF_TXD_TXEN_n |
		BUF_RXD_RXEN_n |
		BUF_PPS_TXEN_n |
		0
	);

	tca9534_set_out(LDRV_TCA9534_ADDR,
		LDRV_RXD_RE_n |
#if 0 
		/* We don't actually TX anything to the module and
		 * this burns power ... */
		LDRV_RXD_DE |
#endif
		0
	);

	/* Configure GPIO alt functions */
		/* Aux UART RX */
	gpio_sfn(0, true);

		/* Aux UART TX */
	gpio_sfn(1, true);

		/* PPS input */
	gpio_dir(2, false);
	gpio_sfn(2, true);
}
