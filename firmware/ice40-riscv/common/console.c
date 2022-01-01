/*
 * console.c
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdint.h>

#include "config.h"
#include "mini-printf.h"


struct wb_uart {
	uint32_t data;
	uint32_t clkdiv;
} __attribute__((packed,aligned(4)));

static volatile struct wb_uart * const uart_regs = (void*)(UART_BASE);


static char _printf_buf[128];
#define SYS_CLK_FREQ 30720000
#define UART_DIV(baud) ((SYS_CLK_FREQ+(baud)/2)/(baud)-2)

void console_init(void)
{
#ifdef BOARD_E1_TRACER
	uart_regs->clkdiv = 22;	/* ~1 Mbaud with clk=24MHz */
#else
	uart_regs->clkdiv = UART_DIV(115200);	/* ~1 Mbaud with clk=30.72MHz */
#endif
}

char getchar(void)
{
	int32_t c;
	do {
		c = uart_regs->data;
	} while (c & 0x80000000);
	return c;
}

int getchar_nowait(void)
{
	int32_t c;
	c = uart_regs->data;
	return c & 0x80000000 ? -1 : (c & 0xff);
}

void putchar(char c)
{
	uart_regs->data = c;
}

void puts(const char *p)
{
	char c;
	while ((c = *(p++)) != 0x00) {
		if (c == '\n')
			uart_regs->data = '\r';
		uart_regs->data = c;
	}
}

int printf(const char *fmt, ...)
{
        va_list va;
        int l;

        va_start(va, fmt);
        l = mini_vsnprintf(_printf_buf, 128, fmt, va);
        va_end(va);

	puts(_printf_buf);

	return l;
}
