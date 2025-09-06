/**
 * Copyright (c) 2025 Steve Chang
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPI_H
#define SPI_H

#include "hardware/spi.h"
#include "gpio.h"

#include <stdbool.h>
#include <stdint.h>

#define BUF_SIZE        (4096)
#define SPI_SPEED       (1000 * 1000)   // 1MHz, 1000KHz

struct gpio_spi_pin
{
    uint8_t sck;    // serial clock
    uint8_t tx;     // master: mo, slave: so
    uint8_t rx;     // master: mi, slave: si
    uint8_t csn;    // chip select/slave select (active low)
};

struct spi_config
{
    spi_inst_t *spi;
    uint32_t spi_speed;
    struct gpio_spi_pin pin;
    bool slave_mode;
};

void cs_select(uint cs_pin);
void cs_deselect(uint cs_pin);
int driver_spi_init(const struct spi_config *spi_cfg);
void spi_master_test();
void spi_slave_test();

#endif  // ~ SPI_MASTER_H
