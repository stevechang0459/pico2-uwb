#include "dw1000.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "spi.h"
#include "led.h"
#include "print.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct dw1000_reg dw1000_regs[] =
{
    {.reg_file_id = DW1000_DEV_ID,     .length = 4,    .reg_file_type = DW1000_RO,  .mnemonic = "DEV_ID",     .desc = "Device Identifier"},
    {.reg_file_id = DW1000_EUI,        .length = 8,    .reg_file_type = DW1000_RW,  .mnemonic = "EUI",        .desc = "Extended Unique Identifier"},
    {.reg_file_id = 0x02, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_PANADR,     .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "PANADR",     .desc = "PAN Identifier and Short Address"},
    {.reg_file_id = DW1000_SYS_CFG,    .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "SYS_CFG",    .desc = "System Configuration bitmap"},
    {.reg_file_id = 0x05, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_SYS_TIME,   .length = 5,    .reg_file_type = DW1000_RO,  .mnemonic = "SYS_TIME",   .desc = "System Time Counter"},
    {.reg_file_id = 0x07, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_TX_FCTRL,   .length = 5,    .reg_file_type = DW1000_RW,  .mnemonic = "TX_FCTRL",   .desc = "Transmit Frame Control"},
    {.reg_file_id = DW1000_TX_BUFFER,  .length = 1024, .reg_file_type = DW1000_WO,  .mnemonic = "TX_BUFFER",  .desc = "Transmit Data Buffer"},
    {.reg_file_id = DW1000_DX_TIME,    .length = 5,    .reg_file_type = DW1000_RW,  .mnemonic = "DX_TIME",    .desc = "Delayed Send or Receive Time"},
    {.reg_file_id = 0x0b, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_RX_FWTO,    .length = 2,    .reg_file_type = DW1000_RW,  .mnemonic = "RX_FWTO",    .desc = "Receive Frame Wait Timeout Period"},
    {.reg_file_id = DW1000_SYS_CTRL,   .length = 4,    .reg_file_type = DW1000_SRW, .mnemonic = "SYS_CTRL",   .desc = "System Control Register"},
    {.reg_file_id = DW1000_SYS_MASK,   .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "SYS_MASK",   .desc = "System Event Mask Register"},
    {.reg_file_id = DW1000_SYS_STATUS, .length = 5,    .reg_file_type = DW1000_SRW, .mnemonic = "SYS_STATUS", .desc = "System Event Status Register"},
    {.reg_file_id = DW1000_RX_FINFO,   .length = 4,    .reg_file_type = DW1000_ROD, .mnemonic = "RX_FINFO",   .desc = "RX Frame Information"},
    {.reg_file_id = DW1000_RX_BUFFER,  .length = 1024, .reg_file_type = DW1000_ROD, .mnemonic = "RX_BUFFER",  .desc = "Receive Data"},
    {.reg_file_id = DW1000_RX_FQUAL,   .length = 8,    .reg_file_type = DW1000_ROD, .mnemonic = "RX_FQUAL",   .desc = "Rx Frame Quality information"},
    {.reg_file_id = DW1000_RX_TTCKI,   .length = 4,    .reg_file_type = DW1000_ROD, .mnemonic = "RX_TTCKI",   .desc = "Receiver Time Tracking Interval"},
    {.reg_file_id = DW1000_RX_TTCKO,   .length = 5,    .reg_file_type = DW1000_ROD, .mnemonic = "RX_TTCKO",   .desc = "Receiver Time Tracking Offset"},
    {.reg_file_id = DW1000_RX_TIME,    .length = 14,   .reg_file_type = DW1000_ROD, .mnemonic = "RX_TIME",    .desc = "Receive Message Time of Arrival"},
    {.reg_file_id = 0x16, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_TX_TIME,    .length = 10,   .reg_file_type = DW1000_RO,  .mnemonic = "TX_TIME",    .desc = "Transmit Message Time of Sending"},
    {.reg_file_id = DW1000_TX_ANTD,    .length = 2,    .reg_file_type = DW1000_RW,  .mnemonic = "TX_ANTD",    .desc = "16-bit Delay from Transmit to Antenna"},
    {.reg_file_id = DW1000_SYS_STATE,  .length = 5,    .reg_file_type = DW1000_RO,  .mnemonic = "SYS_STATE",  .desc = "System State information"},
    {.reg_file_id = DW1000_ACK_RESP_T, .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "ACK_RESP_T", .desc = "Acknowledgement Time and Response Time"},
    {.reg_file_id = 0x1b, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x1c, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_RX_SNIFF,   .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "RX_SNIFF",   .desc = "Pulsed Preamble Reception Configuration"},
    {.reg_file_id = DW1000_TX_POWER,   .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "TX_POWER",   .desc = "TX Power Control"},
    {.reg_file_id = DW1000_CHAN_CTRL,  .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "CHAN_CTRL",  .desc = "Channel Control"},
    {.reg_file_id = 0x20, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_USR_SFD,    .length = 41,   .reg_file_type = DW1000_RW,  .mnemonic = "USR_SFD",    .desc = "User-specified short/long TX/RX SFD sequences"},
    {.reg_file_id = 0x22, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_AGC_CTRL,   .length = 33,   .reg_file_type = DW1000_RW,  .mnemonic = "AGC_CTRL",   .desc = "Automatic Gain Control configuration"},
    {.reg_file_id = DW1000_EXT_SYNC,   .length = 12,   .reg_file_type = DW1000_RW,  .mnemonic = "EXT_SYNC",   .desc = "External synchronisation control"},
    {.reg_file_id = DW1000_ACC_MEM,    .length = 4064, .reg_file_type = DW1000_RO,  .mnemonic = "ACC_MEM",    .desc = "Read access to accumulator data"},
    {.reg_file_id = DW1000_GPIO_CTRL,  .length = 44,   .reg_file_type = DW1000_RW,  .mnemonic = "GPIO_CTRL",  .desc = "Peripheral register bus 1 access"},
    {.reg_file_id = DW1000_DRX_CONF,   .length = 44,   .reg_file_type = DW1000_RW,  .mnemonic = "DRX_CONF",   .desc = "Digital Receiver configuration"},
    {.reg_file_id = DW1000_RF_CONF,    .length = 58,   .reg_file_type = DW1000_RW,  .mnemonic = "RF_CONF",    .desc = "Analog RF Configuration"},
    {.reg_file_id = 0x29, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_TX_CAL,     .length = 52,   .reg_file_type = DW1000_RW,  .mnemonic = "TX_CAL",     .desc = "Transmitter calibration block"},
    {.reg_file_id = DW1000_FS_CTRL,    .length = 21,   .reg_file_type = DW1000_RW,  .mnemonic = "FS_CTRL",    .desc = "Frequency synthesiser control block"},
    {.reg_file_id = DW1000_AON,        .length = 12,   .reg_file_type = DW1000_RW,  .mnemonic = "AON",        .desc = "Always-On register set"},
    {.reg_file_id = DW1000_OTP_IF,     .length = 18,   .reg_file_type = DW1000_RW,  .mnemonic = "OTP_IF",     .desc = "One Time Programmable Memory Interface"},
    {.reg_file_id = DW1000_LDE_CTRL,   .length = 0,    .reg_file_type = DW1000_RW,  .mnemonic = "LDE_CTRL",   .desc = "Leading edge detection control block"},
    {.reg_file_id = DW1000_DIG_DIAG,   .length = 41,   .reg_file_type = DW1000_RW,  .mnemonic = "DIG_DIAG",   .desc = "Digital Diagnostics Interface"},
    {.reg_file_id = 0x30, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x31, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x32, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x33, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x34, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x35, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_PMSC,       .length = 48,   .reg_file_type = DW1000_RW,  .mnemonic = "PMSC",       .desc = "Power Management System Control Block"},
    {.reg_file_id = 0x37, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x38, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x39, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x3a, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x3b, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x3c, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x3d, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x3e, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x3f, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    NULL
};

uint8_t m_tx_buf[4096];
uint8_t m_rx_buf[4096];

int dw1000_non_indexed_read(uint8_t reg_file_id, void *buf, size_t len)
{
    if ((buf == NULL) || (reg_file_id > 0x3F) || (len < 2) || (len + 1 > BUF_SIZE))
        goto err;

    union dw1000_tran_header1 header = {
        .rid = reg_file_id,
        .si  = 0,
        .op  = dw1000_SPI_READ,
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
    printf("%s failed\n", __FUNCTION__);
    return -1;
}

int dw1000_non_indexed_write(uint8_t reg_file_id, void *buf, size_t len)
{
    if ((buf == NULL) || (reg_file_id > 0x3F) || (len < 2) || (len + 1 > BUF_SIZE))
        goto err;

    union dw1000_tran_header1 header = {
        .rid = reg_file_id,
        .si  = 0,
        .op  = dw1000_SPI_WRITE,
    };

    memset(m_tx_buf, 0, len + 1);
    m_tx_buf[0] = header.value;
    memcpy(m_tx_buf + 1, buf, len);

    cs_select(SPI0_CSN);
    spi_write_blocking(SPI_BUS, m_tx_buf, len + 1);
    cs_deselect(SPI0_CSN);

    return 0;
err:
    printf("%s failed\n", __FUNCTION__);
    return -1;
}

int dw1000_short_indexed_read(uint8_t reg_file_id, uint8_t sub_addr, void *buf, size_t len)
{
    if ((buf == NULL) || (reg_file_id > 0x3F) || (sub_addr > 0x7F) || (len + 2 > BUF_SIZE))
        goto err;

    union dw1000_tran_header2 header = {
        .rid      = reg_file_id,
        .si       = 1,
        .op       = dw1000_SPI_READ,
        .sub_addr = sub_addr,
        .ext      = 0,
    };

    memset(m_tx_buf, 0, len + 2);
    memset(m_rx_buf, 0, len + 2);
    m_tx_buf[0] = header.value & 0xFF;
    m_tx_buf[1] = header.value >> 8;

    cs_select(SPI0_CSN);
    spi_write_read_blocking(SPI_BUS, m_tx_buf, m_rx_buf, len + 2);
    cs_deselect(SPI0_CSN);
    memcpy(buf, m_rx_buf + 2, len);

    return 0;
err:
    printf("%s failed\n", __FUNCTION__);
    return -1;
}

int driver_dw1000_spi_init(struct spi_cfg *cfg)
{
    printf("%s\n", __FUNCTION__);

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

    return 0;
err:
    printf("%s failed\n", __FUNCTION__);
    return -1;
}

/**
 * @brief In order to transmit, the host controller must write data for transmission
 * to Register file: 0x09 – Transmit Data Buffer.
 */
int dw1000_prepare_tx_buffer(void *buf, size_t len)
{
    if (len > dw1000_regs[DW1000_TX_BUFFER].length)
        return -1;

    dw1000_non_indexed_write(DW1000_TX_BUFFER, buf, len);
}

int dw1000_set_tx_para()
{
    // TBD
}

int dw1000_tx_message()
{
    int ret;
    union dw1000_reg_sys_ctrl sys_ctrl;
    ret = dw1000_non_indexed_read(DW1000_SYS_CTRL, &sys_ctrl, sizeof(sys_ctrl));
    if (ret < 0)
        goto err;

    sys_ctrl.txstrt = 1;

    ret = dw1000_non_indexed_write(DW1000_SYS_CTRL, &sys_ctrl, sizeof(sys_ctrl));
    if (ret < 0)
        goto err;

    return 0;
err:
    printf("%s failed\n", __FUNCTION__);
    return -1;
}

int dw1000_reg_list_check()
{
    printf("%s\n", __FUNCTION__);

    int i = 0;
    for (const struct dw1000_reg *reg = dw1000_regs; reg->mnemonic != NULL; reg++) {
        if (i != reg->reg_file_id) {
            printf("%s failed: %x != %x\n", __FUNCTION__, i, reg->reg_file_id);
            goto err;
        }
        size_t size;
        switch (reg->reg_file_id) {
        case DW1000_SYS_CFG:
            // size = sizeof(union dw1000_reg_tx_fctrl);
            // if (reg->length != size) {
            //     printf("dw1000_reg_tx_fctrl err: %x != %x\n", reg->length, size);
            //     goto err;
            // }
            break;
        case DW1000_TX_FCTRL:
            size = sizeof(union dw1000_reg_tx_fctrl);
            if (reg->length != size) {
                printf("dw1000_reg_tx_fctrl err: %x != %x\n", reg->length, size);
                goto err;
            }
            break;
        };
        i++;
    }

    return 0;
err:
    return -1;
}

#if (CONFIG_SPI_MASTER_MODE)
void dw1000_spi_master_test()
{
    printf("%s\n", __FUNCTION__);

    int ret;
    struct spi_cfg cfg = {
        .spi = SPI_INST,
        .spi_speed = SPI_SPEED,
        .slave_mode = false,
    };
    ret = driver_dw1000_spi_init(&cfg);
    if (ret < 0)
        return;

    ret = dw1000_reg_list_check();
    if (ret < 0)
        return;

    uint8_t tx_buf[BUF_SIZE], rx_buf[BUF_SIZE];
    memset(tx_buf, 0, sizeof(tx_buf));
    memset(rx_buf, 0, sizeof(rx_buf));

    static bool led_out = 0;
    for (size_t i = 0; ; ++i) {
        printf("Transaction #%d\n", i);
        for (const struct dw1000_reg *reg = dw1000_regs; reg->mnemonic != NULL; reg++) {
            if (((reg->reg_file_type != DW1000_RO) && (reg->reg_file_type != DW1000_RW)) || (reg->length > 64) || (reg->length == 0))
                continue;

            memset(rx_buf, 0, reg->length);
            dw1000_non_indexed_read(reg->reg_file_id, rx_buf, reg->length);
            print_buf(rx_buf, reg->length, reg->desc);

            switch (reg->reg_file_id) {
            case DW1000_DEV_ID:
                union dw1000_reg_dev_id *dev_id = (void *)rx_buf;
                printf("dev_id->value               : %08x\n", dev_id->value);
                printf("dev_id->rev                 : %x\n", dev_id->rev);
                printf("dev_id->ver                 : %x\n", dev_id->ver);
                printf("dev_id->model               : %x\n", dev_id->model);
                printf("dev_id->ridtag              : %x\n", dev_id->ridtag);
                break;

            case DW1000_EUI:
                tx_buf[0] = 0x00;
                tx_buf[1] = 0x00;
                tx_buf[2] = 0x00;
                tx_buf[3] = 0x00;
                tx_buf[4] = 0x00;
                tx_buf[5] = 0x3a;
                tx_buf[6] = 0x66;
                tx_buf[7] = 0xdc;
                dw1000_non_indexed_write(reg->reg_file_id, tx_buf, reg->length);
                memset(rx_buf, 0, reg->length);
                dw1000_non_indexed_read(reg->reg_file_id, rx_buf, reg->length);
                print_buf(rx_buf, reg->length, reg->desc);
                break;

            case DW1000_PANADR:
                union dw1000_reg_panadr *panadr = (void *)rx_buf;
                printf("panadr->short_addr          : %x\n", panadr->short_addr);
                printf("panadr->pan_id              : %x\n", panadr->pan_id);
                break;

            case DW1000_SYS_CFG:
                union dw1000_reg_sys_cfg *sys_cfg = (void *)rx_buf;
                printf("sys_cfg->value              : %08x\n", sys_cfg->value);
                printf("sys_cfg->ffen               : %x\n", sys_cfg->ffen);
                printf("sys_cfg->ffbc               : %x\n", sys_cfg->ffbc);
                printf("sys_cfg->ffab               : %x\n", sys_cfg->ffab);
                printf("sys_cfg->ffad               : %x\n", sys_cfg->ffad);
                printf("sys_cfg->ffaa               : %x\n", sys_cfg->ffaa);
                printf("sys_cfg->ffam               : %x\n", sys_cfg->ffam);
                printf("sys_cfg->ffar               : %x\n", sys_cfg->ffar);
                printf("sys_cfg->ffa4               : %x\n", sys_cfg->ffa4);
                printf("sys_cfg->ffa5               : %x\n", sys_cfg->ffa5);
                printf("sys_cfg->hirq_pol           : %x\n", sys_cfg->hirq_pol);
                printf("sys_cfg->spi_edge           : %x\n", sys_cfg->spi_edge);
                printf("sys_cfg->dis_fce            : %x\n", sys_cfg->dis_fce);
                printf("sys_cfg->dis_drxb           : %x\n", sys_cfg->dis_drxb);
                printf("sys_cfg->dis_phe            : %x\n", sys_cfg->dis_phe);
                printf("sys_cfg->dis_rsde           : %x\n", sys_cfg->dis_rsde);
                printf("sys_cfg->fcs_init2f         : %x\n", sys_cfg->fcs_init2f);
                printf("sys_cfg->dis_phe            : %x\n", sys_cfg->dis_phe);
                printf("sys_cfg->phr_mode           : %x\n", sys_cfg->phr_mode);
                printf("sys_cfg->dis_stxp           : %x\n", sys_cfg->dis_stxp);
                printf("sys_cfg->rxm110k            : %x\n", sys_cfg->rxm110k);
                printf("sys_cfg->rxwtoe             : %x\n", sys_cfg->rxwtoe);
                printf("sys_cfg->rxautr             : %x\n", sys_cfg->rxautr);
                printf("sys_cfg->autoack            : %x\n", sys_cfg->autoack);
                printf("sys_cfg->aackpend           : %x\n", sys_cfg->aackpend);
                break;

            case DW1000_TX_FCTRL:
                union dw1000_reg_tx_fctrl *tx_fctrl = (void *)rx_buf;
            printf("Transmit Frame Length           : %d bytes\n", tx_fctrl->ofs_00.tflen);
                printf("tx_fctrl->ofs_00.tfle       : %x\n", tx_fctrl->ofs_00.tfle);
                printf("tx_fctrl->ofs_00.r          : %x\n", tx_fctrl->ofs_00.r);
                uint16_t txbr[4] = {
                    [0]  = 110,
                    [1]  = 850,
                    [2]  = 6800,
                };
                printf("PRF                         : %d kbps\n", txbr[tx_fctrl->ofs_00.txbr]);
                printf("tx_fctrl->ofs_00.tr         : %x\n", tx_fctrl->ofs_00.tr);
                uint8_t prf[4] = {
                    [0] = 4,
                    [1] = 16,
                    [2] = 64,
                };
                printf("PRF                         : %d MHz\n", prf[tx_fctrl->ofs_00.txprf]);
                uint16_t txpsr[16] = {
                    [4]  = 64,
                    [5]  = 128,
                    [6]  = 256,
                    [7]  = 512,
                    [8]  = 1024,
                    [9]  = 1536,
                    [10] = 2048,
                    [12] = 4096,
                };
                printf("Preamble Length             : %d bytes\n", txpsr[tx_fctrl->ofs_00.pe << 2 | tx_fctrl->ofs_00.txpsr]);
                printf("tx_fctrl->ofs_00.txboffs    : %x\n", tx_fctrl->ofs_00.txboffs);
                printf("tx_fctrl->ofs_04.ifsdelay   : %x\n", tx_fctrl->ofs_04.ifsdelay);
                break;
            };
        }
        pico_set_led(led_out);
        led_out = !led_out;
        sleep_ms(1000);
    }
}
#endif
