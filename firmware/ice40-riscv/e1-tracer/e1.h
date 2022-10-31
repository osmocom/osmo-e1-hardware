/*
 * e1.h
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

void e1_init();
void e1_start(int chan);
void e1_stop(int chan);

void e1_poll(void);

void e1_debug_print(bool data);

volatile uint8_t *e1_data_ptr(int mf, int frame, int ts);
unsigned int e1_data_ofs(int mf, int frame, int ts);
