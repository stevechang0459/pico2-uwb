/**
 * Copyright (c) 2025 Steve Chang
 *
 * SPDX-License-Identifier: MIT
 */

#include "print.h"

#include <stdarg.h>

#define BUF_LEN         0x100

void print_buf(const void *buf, size_t len, const char *fmt, ...) {
    size_t i;

    if (fmt) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }

    const uint8_t *c = buf;

    for (i = 0; i < len; ++i) {
        if (i % 16 == 15)
            printf("%02x\n", c[i]);
        else
            printf("%02x ", c[i]);
    }

    // append trailing newline if there isn't one
    if (i % 16) {
        putchar('\n');
    }
}
