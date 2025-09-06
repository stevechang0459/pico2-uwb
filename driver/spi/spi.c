/**
 * Copyright (c) 2025 Steve Chang
 *
 * SPDX-License-Identifier: MIT
 */

#include "spi.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "led.h"
#include "print.h"
#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cs_select(uint cs_pin)
{
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

void cs_deselect(uint cs_pin)
{
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}

int driver_spi_init(const struct spi_config *spi_cfg)
{
    if (spi_cfg->spi == NULL || (spi_cfg->spi != spi0 && spi_cfg->spi != spi1))
        goto err;

    spi_init(spi_cfg->spi, spi_cfg->spi_speed);
    spi_set_slave(spi_cfg->spi, !!spi_cfg->slave_mode);
    gpio_set_function(spi_cfg->pin.sck, GPIO_FUNC_SPI);
    gpio_set_function(spi_cfg->pin.tx,  GPIO_FUNC_SPI);
    gpio_set_function(spi_cfg->pin.rx,  GPIO_FUNC_SPI);
    gpio_set_function(spi_cfg->pin.csn, GPIO_FUNC_SPI);

    // Make the SPI pins available to picotool
    // bi_decl(bi_4pins_with_func(SPI0_RX_PIN, SPI0_TX_PIN, SPI0_SCK_PIN, SPI0_CSN_PIN, GPIO_FUNC_SPI));

    return 0;
err:
    return -1;
}

#if (CONFIG_SPI_MASTER_MODE)
void spi_master_test()
{
    struct spi_config spi_cfg = {
        .spi        = SPI_INST,
        .spi_speed  = SPI_SPEED,
        .pin.sck    = SPI0_SCK_PIN,
        .pin.tx     = SPI0_TX_PIN,
        .pin.rx     = SPI0_RX_PIN,
        .pin.csn    = SPI0_CSN_PIN,
        .slave_mode = false,
    };

    driver_spi_init(&spi_cfg);

    uint8_t tx_buf[BUF_SIZE], rx_buf[BUF_SIZE];

    // Initialize output buffer
    memset(tx_buf, 0, sizeof(tx_buf));
    memset(rx_buf, 0, sizeof(rx_buf));

    printf("SPI master says: The following buffer will be written to MOSI endlessly:\n");
    print_buf(tx_buf, 5, NULL);

    bool led_out = 0;
    for (size_t i = 0; ; ++i) {
        // Write the output buffer to MOSI, and at the same time read from MISO.
        spi_write_read_blocking(spi_cfg.spi, tx_buf, rx_buf, 5);

        // Write to stdio whatever came in on the MISO line.
        printf("SPI master read[%d]:\n", i);
        print_buf(rx_buf,  5, NULL);
        memset(rx_buf, 0, sizeof(rx_buf));

        pico_set_led(led_out);
        led_out = !led_out;

        // Sleep for ten seconds so you get a chance to read the output.
        sleep_ms(1000);
    }
}
#endif

#if (CONFIG_SPI_SLAVE_MODE)
void spi_slave_test()
{
    struct spi_config spi_cfg = {
        .spi = SPI_INST,
        .spi_speed = SPI_SPEED,
        .slave_mode = true,
    };

    driver_spi_init(&spi_cfg);

    uint8_t tx_buf[BUF_SIZE], rx_buf[BUF_SIZE];

    // Initialize output buffer
    memset(tx_buf, 0, sizeof(tx_buf));
    memset(rx_buf, 0, sizeof(rx_buf));

    tx_buf[0] = 0x00;
    tx_buf[1] = 0x30;
    tx_buf[2] = 0x01;
    tx_buf[3] = 0xCA;
    tx_buf[4] = 0xDE;

    printf("SPI slave says: When reading from MOSI, the following buffer will be written to MISO:\n");
    print_buf(tx_buf, 5);

    bool led_out = 0;
    for (size_t i = 0; ; ++i) {
        // Write the output buffer to MISO, and at the same time read from MOSI.
        spi_write_read_blocking(spi_default, tx_buf, rx_buf, 5);

        // Write to stdio whatever came in on the MOSI line.
        printf("SPI slave read[%d]:\n", i);
        print_buf(rx_buf, 5);
        memset(rx_buf, 0, sizeof(rx_buf));

        pico_set_led(led_out);
        led_out = !led_out;
    }
}
#endif
