/*
 * e1.h
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

void e1_init(bool clk_mode);
void e1_poll(void);
void e1_debug_print(bool data);

#define E1_ERR_ALIGN	0x01	/* at least one alignment error */
#define E1_ERR_CRC	0x02	/* at least one CRC error */
#define E1_ERR_OVFL	0x04	/* at least one Rx overflow */
#define E1_ERR_UNFL	0x08	/* at least one Tx underflow */
uint32_t e1_get_and_clear_errors(void);

volatile uint8_t *e1_data_ptr(int mf, int frame, int ts);
unsigned int e1_data_ofs(int mf, int frame, int ts);
