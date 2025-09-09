#include "print.h"

#define BUF_LEN         0x100

void print_buf(uint8_t buf[], size_t len, const char *title) {
    size_t i;
    if (title)
        printf("%s:\n", title);

    for (i = 0; i < len; ++i) {
        if (i % 16 == 15)
            printf("%02x\n", buf[i]);
        else
            printf("%02x ", buf[i]);
    }

    // append trailing newline if there isn't one
    if (i % 16) {
        putchar('\n');
    }
}
