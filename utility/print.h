/**
 * Copyright (c) 2025 Steve Chang
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef PRINT_H
#define PRINT_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

void print_buf(const void *buf, size_t len, const char *fmt, ...);

#endif  // ~ PRINT_H
