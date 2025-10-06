#ifndef PRINT_H
#define PRINT_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

void print_buf(uint8_t buf[], size_t len, const char *fmt, ...);

#endif  // ~ PRINT_H
