#include "spi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "print.h"
#include "led.h"

#if (CONFIG_SPI_MASTER_MODE)

// Single octet header of the non-indexed SPI transaction
union dwm1000_tran_header1
{
    struct
    {
        uint8_t rid : 6;                // Register file ID – Range 0x00 to 0x3F (64 locations)
        uint8_t si  : 1;                // Bit = 0, says sub-index is not present
        uint8_t op  : 1;                // Operation: 0 = Read, 1 = Write
    };
    uint8_t value;
};

struct dwm1000_tran_header2
{
    uint16_t rid      : 6;              // Register file ID – Range 0x00 to 0x3F (64 locations)
    uint16_t si       : 1;              // Bit = 1, says sub-index is not present
    uint16_t op       : 1;              // Operation: 0 = Read, 1 = Write
    uint16_t sub_addr : 7;              // 7-bit Register File sub-address, range 0x00 to 0x7F (128 byte locations)
    uint16_t ext      : 1;              // Extended Address: 0 = no
};

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}

enum dwm1000_spi_operation
{
    DWM1000_SPI_READ = 0,
    DWM1000_SPI_WRITE = 1,
};

uint8_t m_tx_buf[4096];
uint8_t m_rx_buf[4096];

int dwm1000_non_sub_index_read(uint8_t reg_file_id, void *buf, size_t len)
{
    if ((buf == NULL) || (reg_file_id > 0x3F) || len < 2)
        goto err;

    union dwm1000_tran_header1 header = {
        .rid = reg_file_id,
        .si = 0,
        .op = DWM1000_SPI_READ,
    };

    memset(m_tx_buf, 0, len + 1);
    memset(m_rx_buf, 0, len + 1);
    m_tx_buf[0] = header.value;

    cs_select(SPI0_CSN);
    spi_write_read_blocking(SPI_BUS, m_tx_buf, m_rx_buf, len + 1);
    cs_deselect(SPI0_CSN);
    memcpy(buf, m_rx_buf + 1, len);
    return 0;
err:
    return -1;
}

int driver_dwm1000_spi_init(struct spi_cfg *cfg)
{
    if (cfg->spi == NULL || (cfg->spi != spi0 && cfg->spi != spi1))
        goto err;

    spi_init(cfg->spi, cfg->spi_speed);
    spi_set_slave(cfg->spi, !!cfg->slave_mode);
    gpio_set_function(SPI0_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_TX,  GPIO_FUNC_SPI);
    gpio_set_function(SPI0_RX,  GPIO_FUNC_SPI);
    // gpio_set_function(SPI0_CSN, GPIO_FUNC_SPI);
    // Make the SPI pins available to picotool
    // bi_decl(bi_4pins_with_func(SPI0_RX, SPI0_TX, SPI0_SCK, SPI0_CSN, GPIO_FUNC_SPI));
    bi_decl(bi_3pins_with_func(SPI0_RX, SPI0_TX, SPI0_SCK, GPIO_FUNC_SPI));

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(SPI0_CSN);
    gpio_put(SPI0_CSN, 1);
    gpio_set_dir(SPI0_CSN, GPIO_OUT);
    // Make the CS pin available to picotool
    bi_decl(bi_1pin_with_name(SPI0_CSN, "SPI CS"));

    printf("%s\n", __FUNCTION__);
    return 0;
err:
    return -1;
}

void dwm1000_spi_master_test()
{
    int ret;
    struct spi_cfg cfg = {
        .spi = SPI_INST,
        .spi_speed = SPI_SPEED,
        .slave_mode = false,
    };

    ret = driver_dwm1000_spi_init(&cfg);
    if (ret < 0)
        return;
    printf("%s\n", __FUNCTION__);

    uint8_t tx_buf[BUF_SIZE], rx_buf[BUF_SIZE];
    memset(tx_buf, 0, sizeof(tx_buf));
    memset(rx_buf, 0, sizeof(rx_buf));

    bool led_out = 0;
    for (size_t i = 0; ; ++i) {
        dwm1000_non_sub_index_read(0x00, rx_buf, 4);

        printf("Transaction #%d\n", i);
        print_buf(rx_buf,  4);
        memset(rx_buf, 0, sizeof(rx_buf));

        pico_set_led(led_out);
        led_out = !led_out;
        sleep_ms(1000);
    }
}

int driver_spi_init(struct spi_cfg *cfg)
{
    if (cfg->spi == NULL || (cfg->spi != spi0 && cfg->spi != spi1))
        goto err;

    spi_init(cfg->spi, cfg->spi_speed);
    spi_set_slave(cfg->spi, !!cfg->slave_mode);
    gpio_set_function(SPI0_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_TX,  GPIO_FUNC_SPI);
    gpio_set_function(SPI0_RX,  GPIO_FUNC_SPI);
    gpio_set_function(SPI0_CSN, GPIO_FUNC_SPI);
    // Make the SPI pins available to picotool
    bi_decl(bi_4pins_with_func(SPI0_RX, SPI0_TX, SPI0_SCK, SPI0_CSN, GPIO_FUNC_SPI));

    return 0;
err:
    return -1;
}

void spi_master_test()
{
    struct spi_cfg cfg = {
        .spi = SPI_INST,
        .spi_speed = SPI_SPEED,
        .slave_mode = false,
    };

    driver_spi_init(&cfg);

    uint8_t tx_buf[BUF_SIZE], rx_buf[BUF_SIZE];

    // Initialize output buffer
    memset(tx_buf, 0, sizeof(tx_buf));
    memset(rx_buf, 0, sizeof(rx_buf));

    printf("SPI master says: The following buffer will be written to MOSI endlessly:\n");
    print_buf(tx_buf, 5);

    bool led_out = 0;
    for (size_t i = 0; ; ++i) {
        // Write the output buffer to MOSI, and at the same time read from MISO.
        spi_write_read_blocking(SPI_BUS, tx_buf, rx_buf, 5);

        // Write to stdio whatever came in on the MISO line.
        printf("SPI master read[%d]:\n", i);
        print_buf(rx_buf,  5);
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
    struct spi_cfg cfg = {
        .spi = SPI_INST,
        .spi_speed = SPI_SPEED,
        .slave_mode = true,
    };

    driver_spi_init(&cfg);

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
