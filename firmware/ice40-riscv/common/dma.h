/*
 * dma.h
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdbool.h>

typedef void (*dma_cb)(void *);

/* Direction
 *  0 is E1  to USB
 *  1 is USB to E1
 */

bool dma_ready(void);
void dma_exec(unsigned addr_e1, unsigned addr_usb, unsigned len, bool dir,
              dma_cb cb_fn, void *cb_data);

bool dma_poll(void);
