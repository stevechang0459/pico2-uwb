/**
 * Copyright (c) 2025 Steve Chang
 *
 * SPDX-License-Identifier: MIT
 */

#include "pico/stdlib.h"
#include <stdio.h>
#include <assert.h>

#include "print.h"
#include "spi.h"
#include "led.h"
#include "dw1000.h"
#include "gpio.h"

int main() {
    stdio_init_all();
    printf("Hello, world\n");

    int rc = pico_led_init();
    hard_assert(rc == PICO_OK);

    uint8_t buf[16];
    for (int i = 0; i < sizeof(buf); i++) {
        buf[i] = i;
    }
    print_buf(buf, 16, NULL);

    for (int i = 0; i < 6; i++) {
        printf("i:%d\n", i);
        pico_set_led(1);
        sleep_ms(LED_DELAY_MS);
        pico_set_led(0);
        sleep_ms(LED_DELAY_MS);
    }

    #if (CONFIG_SPI_MASTER_MODE)
    // spi_master_test();
    dw1000_ctx_init();
    dw1000_unit_test();
    #endif
    #if (CONFIG_SPI_SLAVE_MODE)
    spi_slave_test();
    #endif
}
