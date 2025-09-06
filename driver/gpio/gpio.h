/**
 * Copyright (c) 2025 Steve Chang
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef GPIO_H
#define GPIO_H

#include "hardware/gpio.h"

enum rp2350_gpio
{
    GPIO0 = 0,
    GPIO1 = 1,
    GPIO2,
    GPIO3,
    GPIO4,
    GPIO5,
    GPIO6,
    GPIO7,
    GPIO8,
    GPIO9,
    GPIO10,
    GPIO11,
    GPIO12,
    GPIO13,
    GPIO14,
    GPIO15,
    GPIO16,
    GPIO17,
    GPIO18,
    GPIO19,
    GPIO20,
    GPIO21,
    GPIO22,
    GPIO23,
    GPIO24,
    GPIO25,
    GPIO26,
    GPIO27,
    GPIO28,
};

#define CONFIG_SPI_MASTER_MODE  (1)
#define CONFIG_SPI_SLAVE_MODE   (0)

#define IRQ_PIN         GPIO20

#define RSTn_PIN        GPIO21

// #if (CONFIG_SPI_MASTER_MODE)
#define SPI_INST        spi0
// #define SPI_INST         spi_default // spi0
// #define SPI_INST spi1

#define SPI0_SCK_PIN    GPIO18

// #define SPI0_SCK_PIN    PICO_DEFAULT_SPI_SCK_PIN    // 18
// #define SPI0_SCK_PIN    GPIO2
// #define SPI0_SCK_PIN    GPIO6

#define SPI0_TX_PIN     GPIO19

// #define SPI0_TX_PIN     PICO_DEFAULT_SPI_TX_PIN     // 19
// #define SPI0_TX_PIN     GPIO3
// #define SPI0_TX_PIN     GPIO7

#define SPI0_RX_PIN     GPIO16

// #define SPI0_RX_PIN     PICO_DEFAULT_SPI_RX_PIN     // 16
// #define SPI0_RX_PIN     GPIO0
// #define SPI0_RX_PIN     GPIO4

#define SPI0_CSN_PIN    GPIO17

// #define SPI0_CSN_PIN    PICO_DEFAULT_SPI_CSN_PIN    // 17
// #define SPI0_CSN_PIN    GPIO1
// #define SPI0_CSN_PIN    GPIO5
// #endif

// #if (CONFIG_SPI_SLAVE_MODE)
// #define SPI_INST spi0
// // #define SPI_INST spi1

// // #define SPI0_SCK_PIN    GPIO18
// #define SPI0_SCK_PIN    GPIO2
// // #define SPI0_SCK_PIN    GPIO6

// // #define SPI0_TX_PIN     GPIO19
// #define SPI0_TX_PIN     GPIO3
// // #define SPI0_TX_PIN     GPIO7

// // #define SPI0_RX_PIN     GPIO16
// #define SPI0_RX_PIN     GPIO0
// // #define SPI0_RX_PIN     GPIO4

// // #define SPI0_CSN_PIN    GPIO17
// #define SPI0_CSN_PIN    GPIO1
// // #define SPI0_CSN_PIN    GPIO5
// #endif

struct gpio_config
{
    uint pin;
    uint32_t event_mask;
    bool enabled;
    gpio_irq_callback_t callback;
};

int gpio_irq_init(const struct gpio_config *gpio_cfg);

#endif  // ~ GPIO_H
