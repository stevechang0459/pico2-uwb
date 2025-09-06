#ifndef SPI_H
#define SPI_H

#include "hardware/spi.h"

#include <stdbool.h>
#include <stdint.h>

#define CONFIG_SPI_MASTER_MODE  (1)
#define CONFIG_SPI_SLAVE_MODE   (0)

#define BUF_SIZE        (256)

// #define SPI_BUS         (spi_default)
#define SPI_BUS         (spi0)
#define SPI_SPEED       (1000 * 1000)   // 1MHz, 1000KHz
#define SPI0_CS_PIN     (PICO_DEFAULT_SPI_CSN_PIN)  // 17
#define SPI0_CLK_PIN    (PICO_DEFAULT_SPI_SCK_PIN)  // 18
#define SPI0_MOSI_PIN   (PICO_DEFAULT_SPI_TX_PIN)   // 19, tx
#define SPI0_MISO_PIN   (PICO_DEFAULT_SPI_RX_PIN)   // 16, rx

#if (CONFIG_SPI_MASTER_MODE)
#define SPI_INST spi0
// #define SPI_INST spi1

#define SPI0_SCK    18
// #define SPI0_SCK    2
// #define SPI0_SCK    6

#define SPI0_TX     19
// #define SPI0_TX     3
// #define SPI0_TX     7

#define SPI0_RX     16
// #define SPI0_RX     0
// #define SPI0_RX     4

#define SPI0_CSN    17
// #define SPI0_CSN    1
// #define SPI0_CSN    5
#endif

#if (CONFIG_SPI_SLAVE_MODE)
#define SPI_INST spi0
// #define SPI_INST spi1

// #define SPI0_SCK    18
#define SPI0_SCK    2
// #define SPI0_SCK    6

// #define SPI0_TX     19
#define SPI0_TX     3
// #define SPI0_TX     7

// #define SPI0_RX     16
#define SPI0_RX     0
// #define SPI0_RX     4

// #define SPI0_CSN    17
#define SPI0_CSN    1
// #define SPI0_CSN    5
#endif

struct spix_gpio_pin
{
    uint8_t spi_sck;                    // serial clock
    uint8_t spi_tx;                     // master: mo, slave so
    uint8_t spi_rx;                     // master: mi, slave si
    uint8_t spi_csn;                    // chip select/slave select (active low)
};

struct spi_cfg
{
    spi_inst_t *spi;
    uint32_t spi_speed;
    // struct spix_gpio_pin gpio_pin;
    bool slave_mode;
};

int driver_spi_init(struct spi_cfg *cfg);
void spi_master_test();
void spi_slave_test();

#endif  // ~ SPI_MASTER_H
