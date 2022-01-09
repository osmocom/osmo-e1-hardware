/*
 * utils.h
 *
 * Copyright (C) 2019-2020  Sylvain Munaut <tnt@246tNt.com>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include <stdbool.h>

char *hexstr(void *d, int n, bool space);

void _panic(const char *file, int lineno, const char *fmt, ...);
#define panic(fmt, ...) _panic(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
