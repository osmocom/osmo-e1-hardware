/*
 * i2c.h
 *
 * Copyright (C) 2021-2022  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

void    i2c_start(void);
void    i2c_stop(void);
bool    i2c_write(uint8_t data);
uint8_t i2c_read(bool ack);

bool    i2c_write_reg(uint8_t dev, uint8_t reg, uint8_t  val);
bool    i2c_read_reg (uint8_t dev, uint8_t reg, uint8_t *val);

bool    i2c_probe(uint8_t dev);
void    i2c_scan(void);
