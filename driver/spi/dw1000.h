/**
 * Copyright (c) 2025 Steve Chang
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DW1000_H
#define DW1000_H

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stddef.h>

#include "gpio.h"
#include "spi.h"

#define CONFIG_DW1000_SYS_STS_DEBUG     (0)
#define CONFIG_DW1000_TAG               (1)
#define CONFIG_DW1000_ANCHOR            (0)
#define CONFIG_DW1000_AUTO_RX           (1)
#if (CONFIG_DW1000_ANCHOR)
#define CONFIG_DW1000_ANCHOR_POLLING_MODE (0)
#endif


#define IEEE_802_15_4_BLINK_CCP_64      (0xC5)
// data, PAN ID Compress, 16 bits source address, 16 bits destination address
#define IEEE_802_15_4_FCTRL_RANGE_16    (0x8841)
#define DW1000_PAN_ID                   (0xDECA)

#define FCNTL_IEEE_BLINK_CCP_64         (0xC5)      //!< CCP blink frame control
#define FCNTL_IEEE_BLINK_TAG_64         (0x56)      //!< Tag blink frame control
#define FCNTL_IEEE_BLINK_ANC_64         (0x57)      //!< Anchor blink frame control
#define FCNTL_IEEE_RANGE_16             (0x8841)    //!< Range frame control
#define FCNTL_IEEE_PROVISION_16         (0x8844)    //!< Provision frame control

#define DW1000_TWR_CODE_RNG_INIT        (0x20)
#define DW1000_TWR_CODE_POLL            (0x61)
#define DW1000_TWR_CODE_RESP            (0x50)
#define DW1000_TWR_CODE_FINAL           (0x69)

/**
 * The chipping rate given by the IEEE 802.15.4-2011 standard [1] is 499.2 MHz.
 * DW1000 system clocks are referenced to this frequency.
 */
#define IEEE_802_15_4_2001_CHIPPING_RATE (499200000ULL)

/**
 * A 63.8976 GHz sampling clock is associated with ranging for the IEEE 802.15.4-2011 standard,
 * where a 15.65 picosecond time period is referred to, it is an approximation to the period of this clock.
 *
 * 63.8976 GHz
 * = 128 * 499.2 MHz
 * = approximately 15.65 picoseconds
 */
#define DW1000_SAMPLING_CLOCK           (128ULL * IEEE_802_15_4_2001_CHIPPING_RATE)

/**
 * The chipping rate given by the IEEE 802.15.4-2011 standard [1] is 499.2 MHz.
 * DW1000 system clocks are referenced to this frequency. Where the system clock
 * frequency is given as 125 MHz, this is an approximation to the actual system
 * clock frequency of 124.8 MHz.
 *
 * DW1000_SYS_CLOCK
 * = 124.8 MHz
 * = 499.2 MHz / 4 = IEEE_802_15_4_2001_CHIPPING_RATE / 4
 * = 63.8976 GHz / 512 = DW1000_SAMPLING_CLOCK / 512
 * = approximately 8 nanoseconds
 */
#define DW1000_SYS_CLOCK                (IEEE_802_15_4_2001_CHIPPING_RATE / 4ULL)

// #define DX_TIME_MS(t)                   ((t) * DW1000_SYS_CLOCK / 1000)
// #define DX_TIME_US(t)                   ((t) * DW1000_SYS_CLOCK / 1000000)
// #define DX_TIME_NS(t)                   ((t) > 8 ? (t) * DW1000_SYS_CLOCK / 1000000000 : 0)

/**
 * The low-order 9 bits of the delayed Transmit value programmed into Register file:
 * 0x0A – Delayed Send or Receive Time are ignored giving a time resolution of
 * 8 ns, or more precisely 4 ÷ (499.2×10^6)
 *
 * DW1000_SAMPLING_CLOCK
 * = 63.8976 GHz
 * = 512 * DW1000_SYS_CLOCK
 */
#define DX_TIME_MS(t)                   ((uint64_t)(t) * DW1000_SAMPLING_CLOCK / 1000ULL)
#define DX_TIME_US(t)                   ((uint64_t)(t) * DW1000_SAMPLING_CLOCK / 1000000ULL)
#define DX_TIME_NS(t)                   ((uint64_t)(t) * DW1000_SAMPLING_CLOCK / 1000000000ULL)

/**
 * The Receive Frame Wait Timeout period is a 16-bit field. The units for this
 * parameter are roughly 1μs, (the exact unit is 512 counts of the fundamental
 * 499.2 MHz UWB clock, or 1.026 μs).
 *
 * 0.0672164102564103 s = 67.216 ms
 */
#define RXFWTO_TIME_MS                  ((uint64_t)(t) * (IEEE_802_15_4_2001_CHIPPING_RATE / 512) / 1000ULL)
#define RXFWTO_TIME_US                  ((uint64_t)(t) * (IEEE_802_15_4_2001_CHIPPING_RATE / 512) / 1000000ULL)

#define DW1000_TX_BUFFER_SIZE           (1024)
#define DW1000_RX_BUFFER_SIZE           (1024)

// DW1000 Register File IDs
enum DW1000_REG_FILE_ID
{
    DW1000_DEV_ID     = 0x00,           // 0x00
    DW1000_EUI        = 0x01,           // 0x01
    DW1000_PANADR     = 0x03,           // 0x03
    DW1000_SYS_CFG    = 0x04,           // 0x04
    DW1000_SYS_TIME   = 0x06,           // 0x06
    DW1000_TX_FCTRL   = 0x08,           // 0x08
    DW1000_TX_BUFFER  = 0x09,           // 0x09
    DW1000_DX_TIME    = 0x0a,           // 0x0a
    DW1000_RX_FWTO    = 0x0c,           // 0x0c
    DW1000_SYS_CTRL   = 0x0d,           // 0x0d
    DW1000_SYS_MASK   = 0x0e,           // 0x0e
    DW1000_SYS_STATUS = 0x0f,           // 0x0f
    DW1000_RX_FINFO   = 0x10,           // 0x10
    DW1000_RX_BUFFER  = 0x11,           // 0x11
    DW1000_RX_FQUAL   = 0x12,           // 0x12
    DW1000_RX_TTCKI   = 0x13,           // 0x13
    DW1000_RX_TTCKO   = 0x14,           // 0x14
    DW1000_RX_TIME    = 0x15,           // 0x15
    DW1000_TX_TIME    = 0x17,           // 0x17
    DW1000_TX_ANTD    = 0x18,           // 0x18
    DW1000_SYS_STATE  = 0x19,           // 0x19
    DW1000_ACK_RESP_T = 0x1a,           // 0x1a
    DW1000_RX_SNIFF   = 0x1d,           // 0x1d
    DW1000_TX_POWER   = 0x1e,           // 0x1e
    DW1000_CHAN_CTRL  = 0x1f,           // 0x1f
    DW1000_USR_SFD    = 0x21,           // 0x21
    DW1000_AGC_CTRL   = 0x23,           // 0x23
    DW1000_EXT_SYNC   = 0x24,           // 0x24
    DW1000_ACC_MEM    = 0x25,           // 0x25
    DW1000_GPIO_CTRL  = 0x26,           // 0x26
    DW1000_DRX_CONF   = 0x27,           // 0x27
    DW1000_RF_CONF    = 0x28,           // 0x28
    DW1000_TX_CAL     = 0x2a,           // 0x2a
    DW1000_FS_CTRL    = 0x2b,           // 0x2b
    DW1000_AON        = 0x2c,           // 0x2c
    DW1000_OTP_IF     = 0x2d,           // 0x2d
    DW1000_LDE_CTRL   = 0x2e,           // 0x2e
    DW1000_DIG_DIAG   = 0x2f,           // 0x2f
    DW1000_PMSC       = 0x36,           // 0x36
    DW1000_REG_FILE_ID_MAX = 0x40
};

// DW1000 Sub-Register Offsets
enum DW1000_SUB_REG_OFS
{
    // Register file: 0x23 –AGC configuration and control overview
    DW1000_AGC_RES1     = 0x00,         // 0x00, Reserved area 1
    DW1000_AGC_CTRL1    = 0x02,         // 0x02, AGC Control 1
    DW1000_AGC_TUNE1    = 0x04,         // 0x04, AGC Tuning register 1
    DW1000_AGC_RES2     = 0x06,         // 0x06, Reserved area 2
    DW1000_AGC_TUNE2    = 0x0c,         // 0x0c, AGC Tuning register 2
    DW1000_AGC_RES3     = 0x10,         // 0x10, Reserved area 3
    DW1000_AGC_TUNE3    = 0x12,         // 0x12, AGC Tuning register 3
    DW1000_AGC_RES4     = 0x14,         // 0x14, Reserved area 4
    DW1000_AGC_STAT1    = 0x1e,         // 0x1e, AGC Status

    // Register file: 0x24 – External synchronisation control overview
    DW1000_EC_CTRL      = 0x00,         // External clock synchronisation counter configuration
    DW1000_EC_RXTC      = 0x04,         // External clock counter captured on RMARKER
    DW1000_EC_GOLP      = 0x08,         // External clock offset to first path 1 GHz counter

    // Register file: 0x26 - GPIO control and status overview
    DW1000_GPIO_MODE    = 0x00,
    DW1000_GPIO_DIR     = 0x08,
    DW1000_GPIO_DOUT    = 0x0c,
    DW1000_GPIO_IRQE    = 0x10,
    DW1000_GPIO_ISEN    = 0x14,
    DW1000_GPIO_IMODE   = 0x18,
    DW1000_GPIO_IBES    = 0x1c,
    DW1000_GPIO_ICLR    = 0x20,
    DW1000_GPIO_IDBE    = 0x24,
    DW1000_GPIO_RAW     = 0x28,

    // Register file: 0x27 - Digital receiver configuration overview
    DW1000_DRX_RES1     = 0x00,         // 0x00, Reserved area 1.
    DW1000_DRX_TUNE0b   = 0x02,         // 0x02, Digital Tuning Register 0b.
    DW1000_DRX_TUNE1a   = 0x04,         // 0x04, Digital Tuning Register 1a.
    DW1000_DRX_TUNE1b   = 0x06,         // 0x06, Digital Tuning Register 1b.
    DW1000_DRX_TUNE2    = 0x08,         // 0x08, Digital Tuning Register 2.
    DW1000_DRX_RES2     = 0x0c,         // 0x0c, Reserved area 2.
    DW1000_DRX_SFDTOC   = 0x20,         // 0x20, SFD timeout.
    DW1000_DRX_RES3     = 0x22,         // 0x22, Reserved area 3.
    DW1000_DRX_PRETOC   = 0x24,         // 0x24, Preamble detection timeout.
    DW1000_DRX_TUNE4H   = 0x26,         // 0x26, Digital Tuning Register 4H.
    DW1000_DRX_CAR_INT  = 0x28,         // 0x28, Carrier Recovery Integrator Register.
    DW1000_RXPACC_NOSAT = 0x2c,         // 0x2c, Unsaturated accumulated preamble symbols.

    // Register file: 0x28 - Analog RF configuration block overview
    DW1000_RF_RF_CONF   = 0x00,         // 0x00
    DW1000_RF_RES1      = 0x04,         // 0x04
    DW1000_RF_RXCTRLH   = 0x0b,         // 0x0b
    DW1000_RF_TXCTRL    = 0x0c,         // 0x0c
    DW1000_RF_RES2      = 0x10,         // 0x10
    DW1000_RF_STATUS    = 0x2c,         // 0x2c
    DW1000_LDOTUNE      = 0x30,         // 0x30

    // Register file: 0x2A – Transmitter Calibration block overview
    DW1000_TC_SARC      = 0x00,         // 0x00,Transmitter Calibration – SAR control
    DW1000_TC_SARL      = 0x03,         // 0x03,Transmitter Calibration – Latest SAR readings
    DW1000_TC_SARW      = 0x06,         // 0x06,Transmitter Calibration – SAR readings at last Wake-Up
    DW1000_TC_PG_CTRL   = 0x08,         // 0x08,Transmitter Calibration – Pulse Generator Control
    DW1000_TC_PG_STATUS = 0x09,         // 0x09,Transmitter Calibration – Pulse Generator Status
    DW1000_TC_PGDELAY   = 0x0B,         // 0x0B,Transmitter Calibration – Pulse Generator Delay
    DW1000_TC_PGTEST    = 0x0C,         // 0x0C,Transmitter Calibration – Pulse Generator Test

    // Register file: 0x2B - Frequency synthesiser control block overview
    DW1000_FS_RES1      = 0x00,         // 0x00
    DW1000_FS_PLLCFG    = 0x07,         // 0x07
    DW1000_FS_PLLTUNE   = 0x0b,         // 0x0b
    DW1000_FS_RES2      = 0x0c,         // 0x0c
    DW1000_FS_XTALT     = 0x0e,         // 0x0e
    DW1000_FS_RES3      = 0x0f,         // 0x0f

    // Register file: 0x2C - Always-on system control overview
    DW1000_AON_WCFG     = 0x00,
    DW1000_AON_CTRL     = 0x02,
    DW1000_AON_RDAT     = 0x03,
    DW1000_AON_ADDR     = 0x04,
    DW1000_AON_CFG0     = 0x06,
    DW1000_AON_CFG1     = 0x0a,

    // Register file: 0x2D - OTP Memory Interface overview
    DW1000_OTP_WDAT     = 0x00,         // 0x00, OTP Write Data
    DW1000_OTP_ADDR     = 0x04,         // 0x04, OTP Address
    DW1000_OTP_CTRL     = 0x06,         // 0x06, OTP Control
    DW1000_OTP_STAT     = 0x08,         // 0x08, OTP Status
    DW1000_OTP_RDAT     = 0x0a,         // 0x0a, OTP Read Data
    DW1000_OTP_SRDAT    = 0x0e,         // 0x0e, OTP SR Read Data
    DW1000_OTP_SF       = 0x12,         // 0x12, OTP Special Function

    // Register file: 0x2E – Leading Edge Detection Interface overview
    DW1000_LDE_THRESH   = 0x0000,        // 0x0000, RO, LDE Threshold report
    DW1000_LDE_CFG1     = 0x0806,        // 0x0806, RW, LDE Configuration Register 1
    DW1000_LDE_PPINDX   = 0x1000,        // 0x1000, RO, LDE Peak Path Index
    DW1000_LDE_PPAMPL   = 0x1002,        // 0x1002, RO, LDE Peak Path Amplitude
    DW1000_LDE_RXANTD   = 0x1804,        // 0x1804, RW, LDE Receive Antenna Delay configuration
    DW1000_LDE_CFG2     = 0x1806,        // 0x1806, RW, LDE Configuration Register 2
    DW1000_LDE_REPC     = 0x2804,        // 0x2804, RW, LDE Replica Coefficient configuration

    // Register file: 0x2F - Digital Diagnostics Interface overview
    DW1000_EVC_CTRL     = 0x00,
    DW1000_EVC_PHE      = 0x04,
    DW1000_EVC_RSE      = 0x06,
    DW1000_EVC_FCG      = 0x08,
    DW1000_EVC_FCE      = 0x0a,
    DW1000_EVC_FFR      = 0x0c,
    DW1000_EVC_OVR      = 0x0e,
    DW1000_EVC_STO      = 0x10,
    DW1000_EVC_PTO      = 0x12,
    DW1000_EVC_FWTO     = 0x14,
    DW1000_EVC_TXFS     = 0x16,
    DW1000_EVC_HPW      = 0x18,
    DW1000_EVC_TPW      = 0x1a,
    DW1000_EVC_RES1     = 0x1d,
    DW1000_EVC_TMC      = 0x24,

    // Register file: 0x36 - Power Management and System Control overview
    DW1000_PMSC_CTRL0   = 0x00,         // PMSC Control Register 0
    DW1000_PMSC_CTRL1   = 0x04,         // PMSC Control Register 1
    DW1000_PMSC_RES1    = 0x08,         // PMSC reserved area 1
    DW1000_PMSC_SNOZT   = 0x0c,         // PMSC Snooze Time Register
    DW1000_PMSC_RES2    = 0x10,         // PMSC reserved area 2
    DW1000_PMSC_TXFSEQ  = 0x26,         // PMSC fine grain TX sequencing control
    DW1000_PMSC_LEDC    = 0x28,         // PMSC LED Control Register
};

enum DW1000_REG_FILE_TYPE
{
    DW1000_RO  = 0,
    DW1000_WO  = 1,
    DW1000_RW  = 2,
    DW1000_SRW = 3,
    DW1000_ROD = 4,
    DW1000_RWD = 5,
    DW1000_REG_FILE_TYPE_MAX
};

enum dw1000_spi_operation
{
    dw1000_SPI_READ  = 0,
    dw1000_SPI_WRITE = 1,
};

enum uwb_frame_type
{
    UWB_BEACON_FRAME  = 0,
    UWB_DATA_FRAME    = 1,
    UWB_ACK_FRAME     = 2,
    UWB_MAC_CMD_FRAME = 3,
    UWB_RSVD_FRAME1   = 4,
    UWB_RSVD_FRAME2   = 5,
    UWB_RSVD_FRAME3   = 6,
    UWB_RSVD_FRAME4   = 7,
};

// Bit Rate Selection (Data Rate)
enum dw1000_br_sel
{
    DW1000_BR_110KBPS  = 0,
    DW1000_BR_850KBPS  = 1,
    DW1000_BR_6800KBPS = 2,
    DW1000_BR_RSVD     = 3,
};

// Pulse Repetition Frequency Selection (PRF)
enum dw1000_prf_sel
{
    DW1000_PRF_4MHZ  = 0,
    DW1000_PRF_16MHZ = 1,
    DW1000_PRF_64MHZ = 2,
    DW1000_PRF_RSVD  = 3,
};

// Preamble Symbol Repetitions Selection (Preamble Length)
enum dw1000_psr_sel
{
    DW1000_PSR_64   = 0x1,  // The standard preamble length for the 802.15.4 UWB PHY
    DW1000_PSR_128  = 0x5,
    DW1000_PSR_256  = 0x9,
    DW1000_PSR_512  = 0xd,
    DW1000_PSR_1024 = 0x2,  // The standard preamble length for the 802.15.4 UWB PHY
    DW1000_PSR_1536 = 0x6,
    DW1000_PSR_2048 = 0xa,
    DW1000_PSR_4096 = 0x3,  // The standard preamble length for the 802.15.4 UWB PHY
};

enum dw1000_drx_tune2_val
{
    DW1000_PAC_8_PRF_16MHZ  = 0x311A002D,
    DW1000_PAC_8_PRF_64MHZ  = 0x313B006B,
    DW1000_PAC_16_PRF_16MHZ = 0x331A0052,
    DW1000_PAC_16_PRF_64MHZ = 0x333B00BE,
    DW1000_PAC_32_PRF_16MHZ = 0x351A009A,
    DW1000_PAC_32_PRF_64MHZ = 0x353B015E,
    DW1000_PAC_64_PRF_16MHZ = 0x371A011D,
    DW1000_PAC_64_PRF_64MHZ = 0x373B0296,
};

enum dw1000_chan_sel
{
    DW1000_CHAN_1 = 1,
    DW1000_CHAN_2 = 2,
    DW1000_CHAN_3 = 3,
    DW1000_CHAN_4 = 4,
    DW1000_CHAN_5 = 5,
    DW1000_CHAN_7 = 7,
};

enum dw1000_pcode_sel
{
    // For 16 MHz PRF
    DW1000_PCODE_1  = 1,
    DW1000_PCODE_2  = 2,
    DW1000_PCODE_3  = 3,
    DW1000_PCODE_4  = 4,
    DW1000_PCODE_5  = 5,
    DW1000_PCODE_6  = 6,
    DW1000_PCODE_7  = 7,
    DW1000_PCODE_8  = 8,
    // For 64 MHz PRF
    DW1000_PCODE_9  = 9,
    DW1000_PCODE_10 = 10,
    DW1000_PCODE_11 = 11,
    DW1000_PCODE_12 = 12,
    DW1000_PCODE_17 = 17,
    DW1000_PCODE_18 = 18,
    DW1000_PCODE_19 = 19,
    DW1000_PCODE_20 = 20,
    // For 64 MHz PRF (DPS)
    DW1000_PCODE_13 = 13,
    DW1000_PCODE_14 = 14,
    DW1000_PCODE_15 = 15,
    DW1000_PCODE_16 = 16,
    DW1000_PCODE_21 = 21,
    DW1000_PCODE_22 = 22,
    DW1000_PCODE_23 = 23,
    DW1000_PCODE_24 = 24,
};

enum dw1000_hirq_pol_sel
{
    DW1000_HIRQ_POL_ACTIVE_LOW  = 0,
    DW1000_HIRQ_POL_ACTIVE_HIGH = 1,
};

enum dw1000_ads_state
{
    DW1000_ADS_TWR_STATE_RX_INIT = 0,
    DW1000_ADS_TWR_STATE_TX_INIT = 1,
    DW1000_ADS_TWR_STATE_BLINK,
    DW1000_ADS_TWR_STATE_LISTEN,
    DW1000_ADS_TWR_STATE_RANGING_INIT,
    DW1000_ADS_TWR_STATE_INIT_WAIT,
    DW1000_ADS_TWR_STATE_POLL,
    DW1000_ADS_TWR_STATE_POLL_WAIT,
    DW1000_ADS_TWR_STATE_RESPONSE,
    DW1000_ADS_TWR_STATE_RESPONSE_WAIT,
    DW1000_ADS_TWR_STATE_FINAL,
    DW1000_ADS_TWR_STATE_FINAL_WAIT,
};

#define DW1000_SYS_MASK_IRQS            (1 << 0)
#define DW1000_SYS_MASK_CPLOCK          (1 << 1)
#define DW1000_SYS_MASK_ESYNCR          (1 << 2)
#define DW1000_SYS_MASK_AAT             (1 << 3)
#define DW1000_SYS_MASK_TXFRB           (1 << 4)
#define DW1000_SYS_MASK_TXPRS           (1 << 5)
#define DW1000_SYS_MASK_TXPHS           (1 << 6)
#define DW1000_SYS_MASK_TXFRS           (1 << 7)
#define DW1000_SYS_MASK_RXPRD           (1 << 8)
#define DW1000_SYS_MASK_RXSFDD          (1 << 9)
#define DW1000_SYS_MASK_LDEDONE         (1 << 10)
#define DW1000_SYS_MASK_RXPHD           (1 << 11)
#define DW1000_SYS_MASK_RXPHE           (1 << 12)
#define DW1000_SYS_MASK_RXDFR           (1 << 13)
#define DW1000_SYS_MASK_RXFCG           (1 << 14)
#define DW1000_SYS_MASK_RXFCE           (1 << 15)
#define DW1000_SYS_MASK_RXRFSL          (1 << 16)
#define DW1000_SYS_MASK_RXRFTO          (1 << 17)
#define DW1000_SYS_MASK_LDEERR          (1 << 18)
#define DW1000_SYS_MASK_RSVD            (1 << 19)
#define DW1000_SYS_MASK_RXOVRR          (1 << 20)
#define DW1000_SYS_MASK_RXPTO           (1 << 21)
#define DW1000_SYS_MASK_GPIOIRQ         (1 << 22)
#define DW1000_SYS_MASK_SLP2INIT        (1 << 23)
#define DW1000_SYS_MASK_RFPLL_LL        (1 << 24)
#define DW1000_SYS_MASK_CLKPLL_LL       (1 << 25)
#define DW1000_SYS_MASK_RXSFDTO         (1 << 26)
#define DW1000_SYS_MASK_HPDWARN         (1 << 27)
#define DW1000_SYS_MASK_TXBERR          (1 << 28)
#define DW1000_SYS_MASK_AFFREJ          (1 << 29)
#define DW1000_SYS_MASK_HSRBP           (1 << 30)
#define DW1000_SYS_MASK_ICRBP           (1 << 31)

#define DW1000_SYS_MASK_RXRSCS          (1 << 0)
#define DW1000_SYS_MASK_RXPREJ          (1 << 1)
#define DW1000_SYS_MASK_TXPUTE          (1 << 2)

// #define DW1000_BR                       (DW1000_BR_110KBPS)
// #define DW1000_PCODE                    (DW1000_PCODE_5)
// #define DW1000_PRF                      (DW1000_PRF_16MHZ)
// #define DW1000_PSR                      (DW1000_PSR_4096)
#define DW1000_BR                       (DW1000_BR_6800KBPS)
#define DW1000_PCODE                    (DW1000_PCODE_9)
#define DW1000_PRF                      (DW1000_PRF_64MHZ)
#define DW1000_PSR                      (DW1000_PSR_128)

#pragma push
#pragma pack(1)

// Register file: 0x00 - Device Identifier
union DW1000_REG_DEV_ID
{
    struct
    {
        uint32_t rev    : 4;            // Revision
        uint32_t ver    : 4;            // Version
        uint32_t model  : 8;            // The MODEL identifies the device
        uint32_t ridtag : 16;           // Register Identification Tag
    };
    uint32_t value;
};

// Register file: 0x01 - Extended Unique Identifier
union DW1000_REG_EUI
{
    uint8_t value[8];
};

// Register file: 0x02 - Reserved

// Register file: 0x03 - PAN Identifier and Short Address
union DW1000_REG_PANADR
{
    struct
    {
        uint32_t short_addr : 16;
        uint32_t pan_id     : 16;
    };
    uint32_t value;
};

// Register file: 0x04 - System Configuration
union DW1000_REG_SYS_CFG
{
    struct
    {
        uint32_t ffen       : 1;        // Bit[0] Frame Filtering Enable
        uint32_t ffbc       : 1;        // Bit[1] Frame Filtering Behave as a Coordinator
        uint32_t ffab       : 1;        // Bit[2] Frame Filtering Allow Beacon frame reception
        uint32_t ffad       : 1;        // Bit[3] Frame Filtering Allow Acknowledgment frame reception
        uint32_t ffaa       : 1;        // Bit[4] Frame Filtering Allow MAC command frame reception
        uint32_t ffam       : 1;        // Bit[5] Frame Filtering Allow MAC command frame reception
        uint32_t ffar       : 1;        // Bit[6] Frame Filtering Allow Reserved frame types
        uint32_t ffa4       : 1;        // Bit[7] Frame Filtering Allow frames with frame type field of 4
        //
        uint32_t ffa5       : 1;        // Bit[8] Frame Filtering Allow frames with frame type field of 5
        uint32_t hirq_pol   : 1;        // Bit[9] Host interrupt polarity
        uint32_t spi_edge   : 1;        // Bit[10] SPI data launch edge
        uint32_t dis_fce    : 1;        // Bit[11] Disable frame check error handling
        uint32_t dis_drxb   : 1;        // Bit[12] Disable Double RX Buffer
        uint32_t dis_phe    : 1;        // Bit[13] Disable receiver abort on PHR error
        uint32_t dis_rsde   : 1;        // Bit[14] Disable Receiver Abort on RSD error
        /**
         * Bit[15] This bit allows selection of the initial seed value for the
         * FCS generation and checking function that is set at the start of each
         * frame transmission and reception
         */
        uint32_t fcs_init2f : 1;
        //
        uint32_t phr_mode   : 2;        // Bit[17:16] This configuration allows selection of PHR type to be one of two options.
        uint32_t dis_stxp   : 1;        // Bit[18] Disable Smart TX Power control.
        uint32_t rsvd1      : 3;        // Reserved.
        uint32_t rxm110k    : 1;        // Bit[22] Receiver Mode 110 kbps data rate.
        uint32_t rsvd2      : 5;        // Bit[27:23] Reserved.
        uint32_t rxwtoe     : 1;        // Bit[28] Receive Wait Timeout Enable.
        uint32_t rxautr     : 1;        // Bit[29] Receiver Auto-Re-enable.
        uint32_t autoack    : 1;        // Bit[30] Automatic Acknowledgement Enable.
        uint32_t aackpend   : 1;        // Bit[31] Automatic Acknowledgement Pending bit control.
    };
    uint32_t value;
};

_Static_assert(sizeof(union DW1000_REG_SYS_CFG) == 4, "union DW1000_REG_SYS_CFG must be 4 bytes");

// Register file: 0x05 - Reserved

// Register file: 0x06 - System Time Counter
union DW1000_REG_SYS_TIME
{
    uint8_t sys_time[5];                // System Time Counter (40-bit)
    struct
    {
        uint32_t sys_time_l;
        uint8_t sys_time_h;
    };
    uint8_t value[5];
};

// Register file: 0x07 - Reserved

// REG:08:00 - TX_FCTRL - Transmit Frame Control (Octets 0 to 3, 32-bits)
union DW1000_REG_TX_FCTRL_08_00
{
    struct
    {
        uint32_t tflen   : 7;           // Bit[6:0] Transmit Frame Length
        uint32_t tfle    : 3;           // Bit[9:7] Transmit Frame Length Extension
        uint32_t r       : 3;           // Bit[12:10] Reserved
        uint32_t txbr    : 2;           // Bit[14:13] Transmit Bit Rate
        uint32_t tr      : 1;           // Bit[15] Transmit Ranging enable
        //
        uint32_t txprf   : 2;           // Bit[17:16] Transmit Pulse Repetition Frequency
        uint32_t txpsr   : 2;           // Bit[19:18] Transmit Preamble Symbol Repetitions (PSR)
        uint32_t pe      : 2;           // Bit[21:20] Preamble Extension
        uint32_t txboffs : 10;          // Bit[31:22] Transmit buffer index offset
    };
    // uint32_t value;
};

_Static_assert(sizeof(union DW1000_REG_TX_FCTRL_08_00) == 4, "union DW1000_REG_TX_FCTRL_08_00 must be 4 bytes");

// REG:08:04 - TX_FCTRL - Transmit Frame Control (Octet 4, 8-bits)
union DW1000_REG_TX_FCTRL_08_04
{
    struct
    {
        uint8_t ifsdelay : 8;           // Inter-Frame Spacing.
    };
    uint8_t value;
};

// Register file: 0x08 - Transmit Frame Control
union DW1000_REG_TX_FCTRL
{
    struct
    {
        union DW1000_REG_TX_FCTRL_08_00 ofs_00;
        union DW1000_REG_TX_FCTRL_08_04 ofs_04;
    };
    uint8_t value[5];
};

// Register file: 0x0A - Delayed Send or Receive Time
union DW1000_REG_DX_TIME
{
    struct
    {
        uint32_t dx_time_l;             // Delayed Send or Receive Time (40-bit).
        uint8_t dx_time_h;              // Delayed Send or Receive Time (40-bit).
    };
    uint8_t value[5];
};

// Register file: 0x0B - Reserved

// Register file: 0x0C - Receive Frame Wait Timeout Period
union DW1000_REG_RX_FWTO
{
    struct
    {
        /**
         * Bit[15:0] The Receive Frame Wait Timeout period is a 16-bit field.
         * The units for this parameter are roughly 1μs, (the exact unit is 512
         * counts of the fundamental 499.2 MHz UWB clock, or 1.026 μs).
         *
         * This parameter is specified in units of approximately 1 μs, or 128
         * system clock cycles.
         */
        uint32_t rxfwto : 16;
        uint32_t rsvd   : 16;           // Bit[31:16] Reserved
    };
    uint32_t value;
};

// Register file: 0x0D - System Control Register
union DW1000_REG_SYS_CTRL
{
    struct
    {
        uint32_t sfcst     : 1;         // Suppress auto-FCS Transmission (on this next frame).
        uint32_t txstrt    : 1;         // Transmit Start.
        uint32_t txdlys    : 1;         // Transmitter Delayed Sending.
        uint32_t cansfcs   : 1;         // Cancel Suppression of auto-FCS transmission (on the current frame).
        uint32_t rsvd1     : 2;         // Reserved.
        uint32_t trxoff    : 1;         // Transceiver Off.
        uint32_t wait4resp : 1;         // Wait for Response.
        //
        uint32_t rxenab    : 1;         // Enable Receiver.
        uint32_t rxdlye    : 1;         // Receiver Delayed Enable.
        uint32_t rsvd2     : 14;        // Reserved.
        //
        uint32_t hrbpt     : 1;         // Host Side Receive Buffer Pointer Toggle.
        uint32_t rsvd3     : 7;         // Reserved.
    };
    uint32_t value;
};

_Static_assert(sizeof(union DW1000_REG_SYS_CTRL) == 4, "union DW1000_REG_SYS_CTRL must be 4 bytes");

// Register file: 0x0E - System Event Mask Register
union DW1000_REG_SYS_MASK
{
    struct
    {
        uint32_t rsvd1     : 1;         // Bit[0] Reserved.
        uint32_t mcplock   : 1;         // Bit[1] Mask clock PLL lock event.
        uint32_t mesyncr   : 1;         // Bit[2] Mask external sync clock reset event.
        uint32_t maat      : 1;         // Bit[3] Mask automatic acknowledge trigger event.
        uint32_t mtxfrb    : 1;         // Bit[4] Mask transmit frame begins event.
        uint32_t mtxprs    : 1;         // Bit[5] Mask transmit preamble sent event.
        uint32_t mtxphs    : 1;         // Bit[6] Mask transmit PHY Header Sent event.
        uint32_t mtxfrs    : 1;         // Bit[7] Mask transmit frame sent event.
        //
        uint32_t mrxprd    : 1;         // Bit[8] Mask receiver preamble detected event.
        uint32_t mrxsfdd   : 1;         // Bit[9] Mask receiver SFD detected event.
        uint32_t mldedone  : 1;         // Bit[10] Mask LDE processing done event.
        uint32_t mrxphd    : 1;         // Bit[11] Mask receiver PHY header detect event.
        uint32_t mrxphe    : 1;         // Bit[12] *Mask receiver PHY header error event.
        uint32_t mrxdfr    : 1;         // Bit[13] Mask receiver data frame ready event.
        uint32_t mrxfcg    : 1;         // Bit[14] *Mask receiver FCS good event.
        uint32_t mrxfce    : 1;         // Bit[15] *Mask receiver FCS error event.
        //
        uint32_t mrxrfsl   : 1;         // Bit[16] *Mask receiver Reed Solomon Frame Sync Loss event.
        uint32_t mrxrfto   : 1;         // Bit[17] *Mask Receive Frame Wait Timeout event.
        uint32_t mldeerr   : 1;         // Bit[18] *Mask leading edge detection processing error event.
        uint32_t rsvd2     : 1;         // Bit[19] Reserved.
        uint32_t mrxovrr   : 1;         // Bit[20] *Mask Receiver Overrun event.
        uint32_t mrxpto    : 1;         // Bit[21] *Mask Preamble detection timeout event.
        uint32_t mgpioirq  : 1;         // Bit[22] Mask GPIO interrupt event.
        uint32_t mslp2init : 1;         // Bit[23] Mask SLEEP to INIT event.
        //
        uint32_t mrfpllll  : 1;         // Bit[24] *Mask RF PLL Losing Lock warning event.
        uint32_t mcpllll   : 1;         // Bit[25] *Mask Clock PLL Losing Lock warning event.
        uint32_t mrxsfdto  : 1;         // Bit[26] *Mask Receive SFD timeout event.
        uint32_t mhpdwarn  : 1;         // Bit[27] *Mask Half Period Delay Warning event.
        uint32_t mtxberr   : 1;         // Bit[28] *Mask Transmit Buffer Error event.
        uint32_t maffrej   : 1;         // Bit[29] *Mask Automatic Frame Filtering rejection event.
        uint32_t rsvd3     : 2;         // Bit[31:30] Reserved.
    };
    uint32_t value;
};


#define DW1000_SYS_MASK_MRXPHE          (1 << 12)   // Bit[12] Receiver PHY Header Error.
#define DW1000_SYS_MASK_MRXFCG          (1 << 14)   // Bit[14] Receiver FCS Good.
#define DW1000_SYS_MASK_MRXFCE          (1 << 15)   // Bit[15] Receiver FCS Error.

#define DW1000_SYS_MASK_MRXRFSL         (1 << 16)   // Bit[16] Receiver Reed Solomon Frame Sync Loss.
#define DW1000_SYS_MASK_MRXRFTO         (1 << 17)   // Bit[17] *Receive Frame Wait Timeout.
#define DW1000_SYS_MASK_MLDEERR         (1 << 18)   // Bit[18] *Leading edge detection processing error.
#define DW1000_SYS_MASK_MRXOVRR         (1 << 20)   // Bit[20] *Receiver Overrun.
#define DW1000_SYS_MASK_MRXPTO          (1 << 21)   // Bit[21] *Preamble detection timeout.

#define DW1000_SYS_MASK_MRFPLLLL        (1 << 24)   // Bit[24] *RF PLL Losing Lock.
#define DW1000_SYS_MASK_MCLKPLLLL       (1 << 25)   // Bit[25] *Clock PLL Losing Lock.
#define DW1000_SYS_MASK_MRXSTDTO        (1 << 26)   // Bit[26] *Receive SFD timeout.
#define DW1000_SYS_MASK_MHPDWARN        (1 << 27)   // Bit[27] *Half Period Delay Warning.
#define DW1000_SYS_MASK_MTXBERR         (1 << 28)   // Bit[28] *Transmit Buffer Error.
#define DW1000_SYS_MASK_MAFFREJ         (1 << 29)   // Bit[29] *Automatic Frame Filtering rejection.

// #define DW1000_SYS_STS_MASK ( \
//     DW1000_SYS_MASK_MRXPHE   | DW1000_SYS_MASK_MRXFCG    | DW1000_SYS_MASK_MRXFCE   | \
//     DW1000_SYS_MASK_MRXRFSL  | DW1000_SYS_MASK_MRXRFTO   | DW1000_SYS_MASK_MLDEERR  | \
//     DW1000_SYS_MASK_MRXOVRR  | DW1000_SYS_MASK_MRXPTO    | \
//     DW1000_SYS_MASK_MRFPLLLL | DW1000_SYS_MASK_MCLKPLLLL | DW1000_SYS_MASK_MRXSTDTO | \
//     DW1000_SYS_MASK_MHPDWARN | DW1000_SYS_MASK_MTXBERR   | DW1000_SYS_MASK_MAFFREJ)

#define DW1000_SYS_STS_MASK (DW1000_SYS_MASK_MRXFCG | DW1000_SYS_MASK_MRXRFTO)

// REG:0F:00 - SYS_STATUS - System Status Register (octets 0 to 3)
union DW1000_REG_SYS_STATUS_0F_00
{
    struct
    {
        uint32_t irqs      : 1;         // Bit[0] Interrupt Request Status.
        uint32_t cplock    : 1;         // Bit[1] Clock PLL Lock.
        uint32_t esyncr    : 1;         // Bit[2] External Sync Clock Reset.
        uint32_t aat       : 1;         // Bit[3] Automatic Acknowledge Trigger.
        uint32_t txfrb     : 1;         // Bit[4] Transmit Frame Begins.
        uint32_t txprs     : 1;         // Bit[5] Transmit Preamble Sent.
        uint32_t txphs     : 1;         // Bit[6] Transmit PHY Header Sent.
        uint32_t txfrs     : 1;         // Bit[7] Transmit Frame Sent.
        //
        uint32_t rxprd     : 1;         // Bit[8] Receiver Preamble Detected status.
        uint32_t rxsfdd    : 1;         // Bit[9] Receiver SFD Detected.
        uint32_t ldedone   : 1;         // Bit[10] LDE processing done.
        uint32_t rxphd     : 1;         // Bit[11] Receiver PHY Header Detect.
        uint32_t rxphe     : 1;         // Bit[12] **Receiver PHY Header Error.
        uint32_t rxdfr     : 1;         // Bit[13] Receiver Data Frame Ready.
        uint32_t rxfcg     : 1;         // Bit[14] *Receiver FCS Good.
        uint32_t rxfce     : 1;         // Bit[15] **Receiver FCS Error.
        //
        uint32_t rxrfsl    : 1;         // Bit[16] **Receiver Reed Solomon Frame Sync Loss.
        uint32_t rxrfto    : 1;         // Bit[17] **Receive Frame Wait Timeout.
        uint32_t ldeerr    : 1;         // Bit[18] **Leading edge detection processing error.
        uint32_t rsvd      : 1;         // Bit[19] Reserved.
        uint32_t rxovrr    : 1;         // Bit[20] *Receiver Overrun.
        uint32_t rxpto     : 1;         // Bit[21] *Preamble detection timeout.
        uint32_t gpioirq   : 1;         // Bit[22] GPIO interrupt.
        uint32_t slp2init  : 1;         // Bit[23] SLEEP to INIT.
        //
        uint32_t rfpll_ll  : 1;         // Bit[24] **RF PLL Losing Lock.
        uint32_t clkpll_ll : 1;         // Bit[25] **Clock PLL Losing Lock.
        uint32_t rxsfdto   : 1;         // Bit[26] **Receive SFD timeout.
        uint32_t hpdwarn   : 1;         // Bit[27] **Half Period Delay Warning.
        uint32_t txberr    : 1;         // Bit[28] **Transmit Buffer Error.
        uint32_t affrej    : 1;         // Bit[29] **Automatic Frame Filtering rejection.
        uint32_t hsrbp     : 1;         // Bit[30] Host Side Receive Buffer Pointer.
        uint32_t icrbp     : 1;         // Bit[31] IC side Receive Buffer Pointer.
    };
    uint32_t value;
};

_Static_assert(sizeof(union DW1000_REG_SYS_STATUS_0F_00) == 4, "union DW1000_REG_SYS_STATUS_0F_00 must be 4 bytes");

#define DW1000_SYS_STS_RXDFR        (1 << 13)   // Bit[13] Receiver Data Frame Ready.
#define DW1000_SYS_STS_RXFCG        (1 << 14)   // Bit[14] Receiver FCS Good.

#define DW1000_SYS_STS_RXPHE        (1 << 12)   // Bit[12] Receiver PHY Header Error.
#define DW1000_SYS_STS_RXFCE        (1 << 15)   // Bit[15] Receiver FCS Error.
#define DW1000_SYS_STS_RXFSL        (1 << 16)   // Bit[16] Receiver Reed Solomon Frame Sync Loss.

#define DW1000_SYS_STS_RXRFTO       (1 << 17)   // Bit[17] *Receive Frame Wait Timeout.
#define DW1000_SYS_STS_LDEERR       (1 << 18)   // Bit[18] *Leading edge detection processing error.
#define DW1000_SYS_STS_RXOVRR       (1 << 20)   // Bit[20] *Receiver Overrun.
#define DW1000_SYS_STS_RXPTO        (1 << 21)   // Bit[21] *Preamble detection timeout.

#define DW1000_SYS_STS_RFPLL_LL     (1 << 24)   // Bit[24] *RF PLL Losing Lock.
#define DW1000_SYS_STS_CLKPLL_LL    (1 << 25)   // Bit[25] *Clock PLL Losing Lock.
#define DW1000_SYS_STS_RXSTDTO      (1 << 26)   // Bit[26] *Receive SFD timeout.
#define DW1000_SYS_STS_HPDWARN      (1 << 27)   // Bit[27] *Half Period Delay Warning.
#define DW1000_SYS_STS_TXBERR       (1 << 28)   // Bit[28] *Transmit Buffer Error.
#define DW1000_SYS_STS_AFFREJ       (1 << 29)   // Bit[29] *Automatic Frame Filtering rejection.

#define DW1000_SYS_STS_TXFRB        (1 << 4)    // Bit[4] Transmit Frame Begins.
#define DW1000_SYS_STS_TXPRS        (1 << 5)    // Bit[5] Transmit Preamble Sent.
#define DW1000_SYS_STS_TXPHS        (1 << 6)    // Bit[6] Transmit PHY Header Sent.
#define DW1000_SYS_STS_TXFRS        (1 << 7)    // Bit[7] Transmit Frame Sent.

// REG:0F:04 - SYS_STATUS - System Status Register (octet 4)
union DW1000_REG_SYS_STATUS_0F_04
{
    struct
    {
        uint8_t rxrscs : 1;             // Bit[0] **Receiver Reed-Solomon Correction Status.
        uint8_t rxprej : 1;             // Bit[1] Receiver Preamble Rejection.
        uint8_t txpute : 1;             // Bit[2] **Transmit power up time error.
        uint8_t rsvd   : 5;             // Bit[7:3] Reserved.
    };
    uint8_t value;
};

_Static_assert(sizeof(union DW1000_REG_SYS_STATUS_0F_04) == 1, "union DW1000_REG_SYS_STATUS_0F_04 must be 1 bytes");

#define DW1000_SYS_STS_RXRSCS   (1 << 0)    // Bit[0] Receiver Reed-Solomon Correction Status.
#define DW1000_SYS_STS_RXPREJ   (1 << 1)    // Bit[1] Receiver Preamble Rejection.
#define DW1000_SYS_STS_TXPUTE   (1 << 2)    // Bit[2] Transmit power up time error.

// Register file: 0x0F - System Event Status Register
union DW1000_REG_SYS_STATUS
{
    struct
    {
        union DW1000_REG_SYS_STATUS_0F_00 ofs_00;
        union DW1000_REG_SYS_STATUS_0F_04 ofs_04;
    };
    uint8_t value[5];
};

// Register file: 0x10 - RX Frame Information (in double buffer set)
union DW1000_REG_RX_FINFO
{
    struct
    {
        uint32_t rxflen : 7;            // Bit[6:0] Receive Frame Length.
        uint32_t rxfle  : 3;            // Bit[9:7] Receive Frame Length Extension.
        uint32_t rsvd   : 1;            // Bit[10] Reserved.
        uint32_t rxnspl : 2;            // Bit[12:11] Receive non-standard preamble length.
        uint32_t rxbr   : 2;            // Bit[14:13] Receive Bit Rate report.
        uint32_t rng    : 1;            // Bit[15] Receiver Ranging.
        uint32_t rxprfr : 2;            // Bit[17:16] RX Pulse Repetition Rate report.
        uint32_t rxpsr  : 2;            // Bit[19:18] RX Preamble Repetition.
        uint32_t rxpacc : 12;           // Bit[31:20] Preamble Accumulation Count.
    };
    uint32_t value;
};

// Register file: 0x11 - Receive Data (in double buffer set)

// Register file: 0x12 - Rx Frame Quality information (in double buffer set)
union DW1000_REG_RX_FQUAL
{
    struct
    {
        uint32_t std_noise : 16;        // Bit[15:0] Standard Deviation of Noise.
        uint32_t fp_ampl2  : 16;        // Bit[31:16] First Path Amplitude point 2.
        uint32_t fp_ampl3  : 16;        // Bit[15:0] First Path Amplitude point 3.
        uint32_t cir_pwr   : 16;        // Bit[31:16] Channel Impulse Response Power.
    };
    uint64_t value;
};

_Static_assert(sizeof(union DW1000_REG_RX_FQUAL) == 8, "union DW1000_REG_RX_FQUAL must be 8 bytes");

// Register file: 0x13 - Receiver Time Tracking Interval (in double buffer set)

// Register file: 0x14 - Receiver Time Tracking Offset (in double buffer set)

// Register file: 0x15 - Receive Message Time of Arrival (in double buffer set)
union DW1000_REG_RX_TIME
{
    struct
    {
        uint32_t rx_stamp_l;            // The fully adjusted time of reception (low 32 bits of 40-bit value).
        uint32_t rx_stamp_h : 8;        // The fully adjusted time of reception (high 8 bits of 40-bit value).
        uint32_t fp_index   : 16;       // First path index.
        uint32_t fp_ampl1_l : 8;        // First Path Amplitude point 1 (low 8 bits of 16-bit value).
        uint32_t fp_ampl1_h : 8;        // First Path Amplitude point 1 (high 8 bits of 16-bit value).
        uint32_t rx_rawst_l : 24;       // The Raw Timestamp for the frame (low 24 bits of 40-bit value).
        uint16_t rx_rawst_h;            // The Raw Timestamp for the frame (high 16 bits of 40-bit value).
    };
    uint8_t value[14];
};

_Static_assert(sizeof(union DW1000_REG_RX_TIME) == 14, "union DW1000_REG_RX_TIME must be 14 bytes");

// Register file: 0x16 - Reserved

// Register file: 0x17 - Transmit Time Stamp
union DW1000_REG_TX_TIME_STAMP
{
    struct
    {
        uint32_t tx_stamp_l;            // The fully adjusted time of transmission (low 32 bits of 40-bit value).
        uint32_t tx_stamp_h : 8;        // The fully adjusted time of transmission (high 8 bits of 40-bit value).
        uint32_t tx_rawst_l : 24;       // The Raw Timestamp for the frame (low 24 bits of 40-bit value).
        uint16_t tx_rawst_h;            // The Raw Timestamp for the frame (high 16 bits of 40-bit value).
    };
    uint8_t value[10];
};

// Register file: 0x18 - Transmitter Antenna Delay
union DW1000_REG_TX_ANTD
{
    struct
    {
        uint16_t tx_antdl;              // 16-bit Delay from Transmit to Antenna.
    };
    uint16_t value;
};

// Register file: 0x19 - DW1000 State Information
union DW1000_REG_SYS_STATE
{
    struct
    {
        uint32_t tx_state   : 4;
        uint32_t rsvd1      : 4;
        uint32_t rx_state   : 5;
        uint32_t rsvd2      : 3;
        uint32_t pmsc_state : 4;
        uint32_t rsvd3      : 12;
    };
    uint32_t value;
};

_Static_assert(sizeof(union DW1000_REG_SYS_STATE) == 4, "union DW1000_REG_SYS_STATE must be 4 bytes");

// Register file: 0x1A - Acknowledgement time and response time

// Register file: 0x1B - Reserved

// Register file: 0x1C - Reserved

// Register file: 0x1D - SNIFF Mode
union DW1000_REG_RX_SNIFF
{
    struct
    {
        uint32_t sniff_ont  : 4;        // Bit[3:0] SNIFF Mode ON time. (in units of PAC).
        uint32_t rsvd1      : 4;        // Bit[7:4] Reserved.
        /**
         * Bit[15:8] SNIFF Mode OFF time specified in μs. This parameter is
         * specified in units of approximately 1 μs, or 128 system clock cycles
         * (= 128 * 8 ns = 128 * (1 / DW1000_SYS_CLOCK).
         */
        uint32_t sniff_offt : 8;
        uint32_t rsvd2      : 16;       // Bit[31:16] Reserved.
    };
    uint32_t value;
};

// Register file: 0x1E - Transmit Power Control
union DW1000_REG_TX_POWER
{
    // Smart Transmit Power Control (When DIS_STXP = 0)
    struct
    {
        /**
         * Bit[7:0] This is the normal power setting used for frames that do
         * not fall within the data rate and frame length criteria required for
         * a boost, i.e. the frame duration is more than 0.5 ms.
         *
         * It is also the power setting used for the PHR portion of the frame
         * for all of the other three cases.
         */
        uint32_t boostnorm : 8;
        /**
         * Bit[15:8] This value sets the power applied to the preamble and data
         * portions of the frame during transmission at the 6.8 Mbps data rate
         * for frames that are less than 0.5 ms duration which is determined by
         * the following criteria:
         *
         * -- Preamble Length of 64 symbols and Frame Length of <= 333 bytes.
         * -- Preamble Length of 128 symbols and Frame Length of <= 281 bytes.
         * -- Preamble Length of 256 symbols and Frame Length of <= 166 bytes.
         */
        uint32_t boostp500 : 8;
        /**
         * Bit[23:16] This value sets the power applied to the preamble and data
         * portions of the frame during transmission at the 6.8 Mbps data rate
         * for frames that are less than 0.25 ms duration which is determined by
         * the following criteria:
         *
         * -- Preamble Length of 64 symbols and Frame Length of <= 123 bytes.
         * -- Preamble Length of 128 symbols and Frame Length of <= 67 bytes.
         */
        uint32_t boostp250 : 8;
        /**
         * Bit[31:24] This value sets the power applied to the preamble and data
         * portions of the frame during transmission at the 6.8 Mbps data rate
         * for frames that are less than 0.125 ms duration which is determined
         * by the following criteria:
         *
         * -- Preamble Length of 64 symbols, SFD Length <=16 symbols and Frame Length of <= 15 bytes.
         * -- Preamble Length of 64 symbols, SFD Length <=12 symbols and Frame Length of <= 19 bytes.
         * -- Preamble Length of 64 symbols, SFD Length of 8 symbols and Frame Length of <= 23 bytes.
         */
        uint32_t boostp125 : 8;
    };
    // Manual Transmit Power Control
    struct
    {
        uint32_t na1      : 8;          // Bit[7:0] Not applicable = 0x22
        /**
         * Bit[15:8] This power setting is applied during the transmission of
         * the PHY header (PHR) portion of the frame.
         */
        uint32_t txpowphr : 8;
        /**
         * Bit[23:16] This power setting is applied during the transmission of
         * the synchronisation header (SHR) and data portions of the frame.
         */
        uint32_t txpowsd  : 8;
        uint32_t na2      : 8;          // Bit[31:24] Not applicable = 0x0E
    };
    uint32_t value;
};

_Static_assert(sizeof(union DW1000_REG_TX_POWER) == 4, "union DW1000_REG_TX_POWER must be 4 bytes");

// Register file: 0x1F - Channel Control
union DW1000_REG_CHAN_CTRL
{
    struct
    {
        uint32_t tx_chan  : 4;          // Bit[3:0] Transmit channel.
        uint32_t rx_chan  : 4;          // Bit[7:4] Receive channel.
        uint32_t rsvd     : 9;          // Bit[16:8] Reserved.
        uint32_t dwsfd    : 1;          // Bit[17] Decawave proprietary SFD sequence.
        uint32_t rxprf    : 2;          // Bit[19:18] PRF used in the receiver.
        uint32_t tnssfd   : 1;          // Bit[20] User specified (non-standard) SFD in the transmitter.
        uint32_t rnssfd   : 1;          // Bit[21] User specified (non-standard) SFD in the receiver.
        uint32_t tx_pcode : 5;          // Bit[26:22] Preamble code used in the transmitter.
        uint32_t rx_pcode : 5;          // Bit[31:27] Preamble code used in the receiver.
    };
    uint32_t value;
};

// Register file: 0x20 - Reserved

// Register file: 0x21 - User defined SFD sequence

// Register file: 0x22 - Reserved

// Sub-Register 0x23:00 - AGC_RES1
union DW1000_SUB_REG_AGC_RES1
{
    uint16_t value;
};

_Static_assert(sizeof(union DW1000_SUB_REG_AGC_RES1) == 2, "union DW1000_SUB_REG_AGC_RES1 must be 2 bytes");

// Sub-Register 0x23:02 - AGC_CTRL1
union DW1000_SUB_REG_AGC_CTRL1
{
    struct
    {
        uint16_t dis_am : 1;
        uint16_t rsvd   : 15;
    };
    uint16_t value;
};

_Static_assert(sizeof(union DW1000_SUB_REG_AGC_CTRL1) == 2, "union DW1000_SUB_REG_AGC_CTRL1 must be 2 bytes");

// Sub-Register 0x23:04 - AGC_TUNE1
union DW1000_SUB_REG_AGC_TUNE1
{
    uint16_t value;
};

_Static_assert(sizeof(union DW1000_SUB_REG_AGC_TUNE1) == 2, "union DW1000_SUB_REG_AGC_TUNE1 must be 2 bytes");

// Sub-Register 0x23:06 - AGC_RES2
union DW1000_SUB_REG_AGC_RES2
{
    uint8_t value[6];
};

_Static_assert(sizeof(union DW1000_SUB_REG_AGC_RES2) == 6, "union DW1000_SUB_REG_AGC_RES2 must be 6 bytes");

// Sub-Register 0x23:0C - AGC_TUNE2
union DW1000_SUB_REG_AGC_TUNE2
{
    uint32_t value;
};

_Static_assert(sizeof(union DW1000_SUB_REG_AGC_TUNE2) == 4, "union DW1000_SUB_REG_AGC_TUNE2 must be 2 bytes");

// Sub-Register 0x23:10 - AGC_RES3
union DW1000_SUB_REG_AGC_RES3
{
    uint8_t value[2];
};

_Static_assert(sizeof(union DW1000_SUB_REG_AGC_RES3) == 2, "union DW1000_SUB_REG_AGC_RES3 must be 2 bytes");

// Sub-Register 0x23:12 - AGC_TUNE3
union DW1000_SUB_REG_AGC_TUNE3
{
    uint16_t value;
};

_Static_assert(sizeof(union DW1000_SUB_REG_AGC_TUNE3) == 2, "union DW1000_SUB_REG_AGC_TUNE3 must be 2 bytes");

// Sub-Register 0x23:14 - AGC_RES4
union DW1000_SUB_REG_AGC_RES4
{
    uint8_t value[10];
};

_Static_assert(sizeof(union DW1000_SUB_REG_AGC_RES4) == 10, "union DW1000_SUB_REG_AGC_RES4 must be 10 bytes");

// Sub-Register 0x23:1E - AGC_STAT1
union DW1000_SUB_REG_AGC_STAT1
{
    struct
    {
        uint16_t rsvd1  : 6;            // Bit[5:0] Reserved.
        uint16_t edg1   : 5;            // Bit[10:6] This 5-bit gain value relates to input noise power measurement.
        uint16_t edv2_l : 5;            // Bit[15:11] This 9-bit value relates to the input noise power measurement.
        uint8_t edv2_h  : 4;            // Bit[19:16] This 9-bit value relates to the input noise power measurement.
        uint8_t rsvd2   : 4;            // Bit[23:20] Reserved.
    };
};

_Static_assert(sizeof(union DW1000_SUB_REG_AGC_STAT1) == 3, "union DW1000_SUB_REG_AGC_STAT1 must be 3 bytes");

// Register file: 0x23 - AGC configuration and control
union DW1000_REG_AGC_CTRL
{
    struct
    {
        union DW1000_SUB_REG_AGC_RES1 agc_res1;
        union DW1000_SUB_REG_AGC_CTRL1 agc_ctrl1;
        union DW1000_SUB_REG_AGC_TUNE1 agc_tune1;
        union DW1000_SUB_REG_AGC_RES2 agc_res2;
        union DW1000_SUB_REG_AGC_TUNE2 agc_tune2;
        union DW1000_SUB_REG_AGC_RES3 agc_res3;
        union DW1000_SUB_REG_AGC_TUNE3 agc_tune3;
        union DW1000_SUB_REG_AGC_RES4 agc_res4;
        union DW1000_SUB_REG_AGC_STAT1 agc_stat1;
    };
};

_Static_assert(sizeof(union DW1000_REG_AGC_CTRL) == 33, "union DW1000_REG_AGC_CTRL must be 33 bytes");

// Sub-Register 0x24:00 EC_CTRL, External clock synchronisation counter configuration
union DW1000_SUB_REG_EC_CTRL
{
    struct
    {
        uint32_t ostsm  : 1;            // Bit[0] External transmit synchronisation mode enable.
        uint32_t osrsm  : 1;            // Bit[1] External receive synchronisation mode enable.
        uint32_t pllldt : 1;            // Bit[2] Clock PLL lock detect tune.
        /**
         * Bit[10:3] Wait counter used for external transmit synchronisation and
         * external timebase reset.
         */
        uint32_t wait   : 8;
        uint32_t ostrm  : 1;            // Bit[11] External timebase reset mode enable.
        uint32_t rsvd   : 20;           // Bit[31:12] Reserved.
    };
    uint32_t value;
};

// Sub-Register 0x24:04 EC_RXTC, External clock counter captured on RMARKER
union DW1000_SUB_REG_EC_RXTC
{
    struct
    {
        /**
         * Bit[31:0] External clock synchronisation counter captured on RMARKER.
         */
        uint32_t rx_ts_est;
    };
    uint32_t value;
};

// Sub-Register 0x24:08 EC_GOLP, External clock offset to first path 1 GHz counter
union DW1000_SUB_REG_EC_GOLP
{
    struct
    {
        /**
         * Bit[5:0] This register contains the 1 GHz count from the arrival of
         * the RMARKER and the next edge of the external clock.
         */
        uint32_t offset_ext : 6;
        uint32_t rsvd       : 26;       // Bit[31:6] Reserved.
    };
    uint32_t value;
};

// Register file: 0x24 - External Synchronisation Control
union DW1000_REG_EXT_SYNC
{
    struct
    {
        union DW1000_SUB_REG_EC_CTRL ec_ctrl;
        union DW1000_SUB_REG_EC_RXTC ec_rxtc;
        union DW1000_SUB_REG_EC_GOLP ec_golp;
    };
};

_Static_assert(sizeof(union DW1000_REG_EXT_SYNC) == 12, "union DW1000_REG_EXT_SYNC must be 12 bytes");

// Register file: 0x25 - Accumulator CIR memory

// Sub-Register 0x26:00 - GPIO_MODE
union DW1000_SUB_REG_GPIO_MODE
{
    struct
    {
        uint32_t rsvd1 : 6; // Bit[5:0] Reserved.
        uint32_t msgp0 : 2; // Bit[7:6] Mode Selection for GPIO0/RXOKLED.
        uint32_t msgp1 : 2; // Bit[9:8] Mode Selection for GPIO1/SFDLED.
        uint32_t msgp2 : 2; // Bit[11:10] Mode Selection for GPIO2/RXLED.
        uint32_t msgp3 : 2; // Bit[13:12] Mode Selection for GPIO3/TXLED.
        uint32_t msgp4 : 2; // Bit[15:14] Mode Selection for GPIO4/EXTPA.
        uint32_t msgp5 : 2; // Bit[17:16] Mode Selection for GPIO5/EXTTXE.
        uint32_t msgp6 : 2; // Bit[19:18] Mode Selection for GPIO6/EXTRXE.
        uint32_t msgp7 : 2; // Bit[21:20] Mode Selection for SYNC/GPIO7.
        uint32_t msgp8 : 2; // Bit[23:22] Mode Selection for IRQ/GPIO8.
        uint32_t rsvd2 : 8; // Bit[31:24] Reserved.
    };
    uint32_t value;
};

_Static_assert(sizeof(union DW1000_SUB_REG_GPIO_MODE) == 4, "union DW1000_SUB_REG_GPIO_MODE must be 4 bytes");

// Sub-Register 0x26:04 - Reserved

// Sub-Register 0x26:08 - GPIO_DIR
union DW1000_SUB_REG_GPIO_DIR
{
    uint32_t value;
};

// Sub-Register 0x26:0C - GPIO_DOUT
union DW1000_SUB_REG_GPIO_DOUT
{
    uint32_t value;
};

// Sub-Register 0x26:10 - GPIO_IRQE
union DW1000_SUB_REG_GPIO_IRQE
{
    uint32_t value;
};

// Sub-Register 0x26:14 - GPIO_ISEN
union DW1000_SUB_REG_GPIO_ISEN
{
    uint32_t value;
};

// Sub-Register 0x26:18 - GPIO_IMODE
union DW1000_SUB_REG_GPIO_IMODE
{
    uint32_t value;
};

// Sub-Register 0x26:1C - GPIO_IBES
union DW1000_SUB_REG_GPIO_IBES
{
    uint32_t value;
};

// Sub-Register 0x26:20 - GPIO_ICLR
union DW1000_SUB_REG_GPIO_ICLR
{
    uint32_t value;
};

// Sub-Register 0x26:24 - GPIO_IDBE
union DW1000_SUB_REG_GPIO_IDBE
{
    uint32_t value;
};

// Sub-Register 0x26:28 - GPIO_RAW
union DW1000_SUB_REG_GPIO_RAW
{
    uint32_t value;
};

// Register file: 0x26 - GPIO control and status
union DW1000_REG_GPIO_CTRL
{
    struct
    {
        union DW1000_SUB_REG_GPIO_MODE gpio_mode;
        uint32_t rsvd;
        union DW1000_SUB_REG_GPIO_DIR gpio_dir;
        union DW1000_SUB_REG_GPIO_DOUT gpio_dout;
        union DW1000_SUB_REG_GPIO_IRQE gpio_irqe;
        union DW1000_SUB_REG_GPIO_ISEN gpio_isen;
        union DW1000_SUB_REG_GPIO_IMODE gpio_imode;
        union DW1000_SUB_REG_GPIO_IBES gpio_ibes;
        union DW1000_SUB_REG_GPIO_ICLR gpio_iclr;
        union DW1000_SUB_REG_GPIO_IDBE gpio_idbe;
        union DW1000_SUB_REG_GPIO_RAW gpio_raw;
    };
    uint8_t value[44];
};

/**
 * Sub-Register 0x27:00 - DRX_RES1, Reserved area 1.
 *
 * Register file: 0x27 - Digital receiver configuration, sub-register 0x00 is a
 * reserved area. Please take care not to write to this register as doing so may
 * cause the DW1000 to malfunction.
 */
union DW1000_SUB_REG_DRX_RES1
{
    /**
     * Byte[1:0] Reserved. Please take care not to write to this register as
     * doing so may cause the DW1000 to malfunction.
     */
    uint8_t value[2];
};

// Sub-Register 0x27:02 - DRX_TUNE0b, Digital Tuning Register 0b.
union DW1000_SUB_REG_DRX_TUNE0B
{
    uint16_t value;                     // Bit[15:0] Digital Tuning Register 0b.
};

// Sub-Register 0x27:04 - DRX_TUNE1a, Digital Tuning Register 1a.
union DW1000_SUB_REG_DRX_TUNE1A
{
    uint16_t value;                     // Bit[15:0] Digital Tuning Register 0b.
};

// Sub-Register 0x27:06 - DRX_TUNE1b, Digital Tuning Register 1b.
union DW1000_SUB_REG_DRX_TUNE1B
{
    uint16_t value;                     // Bit[15:0] Digital Tuning Register 0b.
};

// Sub-Register 0x27:08 - DRX_TUNE2, Digital Tuning Register 2.
union DW1000_SUB_REG_DRX_TUNE2
{
    uint32_t value;                     // Bit[31:0] RW, Digital Tuning Register 2.
};

/**
 * Sub-Register 0x27:0C - DRX_RES2, Reserved area 2.
 *
 * Register file: 0x27 - Digital receiver configuration, from offset 0x0C to
 * offset 0x1F inclusive is a reserved area. Please take care not to write to
 * this area as doing so may cause the DW1000 to malfunction.
 */
union DW1000_SUB_REG_DRX_RES2
{
    /**
     * Byte[19:0] Reserved. Please take care not to write to this register as
     * doing so may cause the DW1000 to malfunction.
     */
    uint8_t value[20];
};

// Sub-Register 0x27:20 - DRX_SFDTOC, SFD detection timeout count
union DW1000_SUB_REG_DRX_STDTOC
{
    uint16_t value;                     // Bit[15:0] SFD detection timeout count (in units of preamble symbols).
};

/**
 * Sub-Register 0x27:22 - DRX_RES3, Reserved area 3.
 *
 * Register file: 0x27 - Digital receiver configuration, sub-register 0x22 is a
 * reserved area. Please take care not to write to this register as doing so may
 * cause the DW1000 to malfunction.
 */
union DW1000_SUB_REG_DRX_RES3
{
    /**
     * Byte[1:0] Reserved. Please take care not to write to this register as
     * doing so may cause the DW1000 to malfunction.
     */
    uint8_t value[2];
};

// Sub-Register 0x27:24 - DRX_PRETOC, Preamble detection timeout count
union DW1000_SUB_REG_DRX_PRETOC
{
    uint16_t value;                     // Bit[15:0] Preamble detection timeout count (in units of PAC size symbols).
};

// Sub-Register 0x27:26 - DRX_TUNE4H, Digital Tuning Register 4h.
union DW1000_SUB_REG_DRX_TUNE4H
{
    uint16_t value;                     // Bit[15:0] Digital Tuning Register 4h.
};

// Sub-Register 0x27:28 - DRX_CAR_INT, Carrier Recovery Integrator Register.
union DW1000_SUB_REG_DRX_CAR_INT
{
    /**
     * Bit[20:0] RO, Carrier Recovery Integrator Register
     *
     * Register file: 0x27 - Digital receiver configuration, sub-register 0x28
     * is a read-only 21 bit register.
     */
    uint8_t value[3];
};

// Sub-Register 0x27:2C - RXPACC_NOSAT, Unsaturated accumulated preamble symbols.
union DW1000_SUB_REG_RXPACC_NOSAT
{
    /**
     * Bit[15:0] RO, Digital debug register. Unsaturated accumulated preamble
     * symbols.
     */
    uint16_t value;
};

// Register file: 0x27 - Digital receiver configuration
union DW1000_REG_DRX_CONF
{
    struct
    {
        union DW1000_SUB_REG_DRX_RES1 drx_res1;         // Byte[1:0] Reserved area 1.
        union DW1000_SUB_REG_DRX_TUNE0B drx_tune0b;     // Byte[3:2] Digital Tuning Register 0b.
        union DW1000_SUB_REG_DRX_TUNE1A drx_tune1a;     // Byte[5:4] Digital Tuning Register 1a.
        union DW1000_SUB_REG_DRX_TUNE1B drx_tune1b;     // Byte[7:6] Digital Tuning Register 1b.
        union DW1000_SUB_REG_DRX_TUNE2 drx_tune2;       // Byte[11:8] Digital Tuning Register 2.4
        union DW1000_SUB_REG_DRX_RES2 drx_res2;         // Byte[31:12] Reserved area 2.
        union DW1000_SUB_REG_DRX_STDTOC drx_stdtoc;     // Byte[33:32] SFD detection timeout count.
        union DW1000_SUB_REG_DRX_RES3 drx_res3;         // Byte[35:34] Reserved area 3.
        union DW1000_SUB_REG_DRX_PRETOC drx_pretoc;     // Byte[37:36] Preamble detection timeout count.
        union DW1000_SUB_REG_DRX_TUNE4H drx_tune4h;     // Byte[39:38] Digital Tuning Register.
        union DW1000_SUB_REG_DRX_CAR_INT drx_car_int;   // Byte[42:40] Carrier Recovery Integrator Register.
        uint8_t rsvd;                                   // Byte[43] Reserved.
        union DW1000_SUB_REG_RXPACC_NOSAT rxpacc_nosat; // Byte[45:44] Unsaturated accumulated preamble symbols.
    };
};

_Static_assert(sizeof(union DW1000_REG_DRX_CONF) == 46, "union DW1000_REG_DRX_CONF must be 46 bytes");

// Sub-Register 0x28:00 - RF_CONF, RF Configuration Register
union DW1000_SUB_REG_RF_CONF
{
    struct
    {
        /**
         * Bit[7:0] Reserved. These fields are reserved, and should not be set
         * to 1 (may be overwritten with 0).
         */
        uint32_t rsvd1  : 8;
        uint32_t txfen  : 5;            // Bit[12:8] Transmit block force enable.
        /**
         * Bit[15:13] PLL block force enables. Write 0x5 to enable the CLK_PLL
         * or 0x7 to enable both the CLK_PLL and RF PLL.
         */
        uint32_t pllfen : 3;
        /**
         * Bit[20:16] Write 0x1F to force the enable to all LDO’s.
         */
        uint32_t ldofen : 5;
        /**
         * Bit[22:21] Force the TX/RX switch. To configure for TX the value
         * written should be set to 0x2, and to configure for RX the value
         * should be set to 0x1.
         */
        uint32_t txrxsw : 2;
        /**
         * Bit[31:23] Reserved. These fields are reserved, and should not be set
         * to 1 (may be overwritten with 0).
         */
        uint32_t rsvd2  : 9;
    };
    uint32_t value;
};

/**
 * Sub-Register 0x28:04 - RF_RES1, Reserved area 1
 *
 * Register file: 0x28 - Analog RF configuration block, sub-register 0x04 is a
 * reserved register. Please take care not to write to this register as doing so
 * may cause the DW1000 to malfunction.
 */
union DW1000_SUB_REG_RF_RES1
{
    /**
     * Byte[6:0] Reserved area 1.
     *
     * Please take care not to write to this register as doing so may cause the
     * DW1000 to malfunction.
     */
    uint8_t value[7];
};

// Sub-Register 0x28:0B - RF_RXCTRLH, Analog RX Control Register
union DW1000_SUB_REG_RF_RXCTRLH
{
    uint8_t value;                      // Bit[7:0] Analog RX Control Register
};

/**
 * Sub-Register 0x28:0C - RF_TXCTRL, Analog TX Control Register
 *
 * Default: 0x1e3de0, RF_TXCTRL is not set to the optimum values by default.
 */
union DW1000_SUB_REG_RF_TXCTRL
{
    struct
    {
        uint16_t rsvd1   : 5;           // Bit[4:0] These fields are reserved. Program only as directed in Table 38.
        uint16_t txmtune : 4;           // Bit[8:5] Transmit mixer tuning register.
        uint16_t txmq    : 3;           // Bit[11:9] Transmit mixer Q-factor tuning register.
        uint16_t rsvd2   : 4;           // Bit[15:12] Reserved.
        uint8_t rsvd3;                  // Bit[23:16] Reserved.
    };
    uint8_t value[3];
};

_Static_assert(sizeof(union DW1000_SUB_REG_RF_TXCTRL) == 3, "union DW1000_SUB_REG_RF_TXCTRL must be 3 bytes");

/**
 * Sub-Register 0x28:10 - RF_RES2, Reserved area 2
 *
 * Register file: 0x28 - Analog RF configuration block, sub-register 0x10 is a
 * reserved register. Please take care not to write to this register as doing so
 * may cause the DW1000 to malfunction.
 */
union DW1000_SUB_REG_RF_RES2
{
    /**
     * Byte[15:0] Reserved area 2.
     *
     * Please take care not to write to this register as doing so may cause the
     * DW1000 to malfunction.
     */
    uint8_t value[16];
};

// Sub-Register 0x28:2C - RF_STATUS, RF Status Register
union DW1000_SUB_REG_RF_STATUS
{
    struct
    {
        uint32_t cplllock  : 1;         // Bit[0] Clock PLL Lock status.
        uint32_t cplllow   : 1;         // Bit[1] Clock PLL Low flag status bit.
        uint32_t cpllhigh  : 1;         // Bit[2] Clock PLL High flag status bit.
        uint32_t rfplllock : 1;         // Bit[3] RF PLL Lock status.
        uint32_t rsvd      : 28;        // Bit[31:4] Reserved.
    };
    uint32_t value;
};

// Sub-Register 0x28:30 - LDOTUNE
union DW1000_SUB_REG_LDOTUNE
{
    /**
     * Byte[4:0] This register is used to control the output voltage levels of
     * the on chip LDOs.
     */
    struct
    {
        uint32_t ldotune_l;
        uint8_t ldotune_h;
    };
    uint8_t value[5];
};

// Register file: 0x28 - Analog RF configuration block
union DW1000_REG_RF_CONF
{
    struct
    {
        union DW1000_SUB_REG_RF_CONF rf_conf;       // Byte[3:0] RF Configuration Register.
        /**
         * Byte[10:4] Reserved area 1. Please take care not to write to this
         * register as doing so may cause the DW1000 to malfunction.
         */
        union DW1000_SUB_REG_RF_RES1 rf_res1;
        union DW1000_SUB_REG_RF_RXCTRLH rf_rxctrlh; // Byte[11] Analog RX Control Register.
        union DW1000_SUB_REG_RF_TXCTRL rf_txctrl;   // Byte[14:12] Analog TX Control Register.
        uint8_t rsvd1;                              // Byte[15] Reserved1.
        union DW1000_SUB_REG_RF_RES2 rf_res2;       // Byte[31:16] Reserved area 2.
        uint8_t rsvd2[12];                          // Byte[43:32] Reserved2.
        union DW1000_SUB_REG_RF_STATUS rf_status;   // Byte[47:44] RF Status Register.
        union DW1000_SUB_REG_LDOTUNE ldotune;       // Byte[52:48] Internal LDO voltage tuning parameter.
    };
    uint8_t value[53];
};

_Static_assert(sizeof(union DW1000_REG_RF_CONF) == 53, "union DW1000_REG_RF_CONF must be 53 bytes");

// Register file: 0x29 - Reserved

// Sub-Register 0x2A:00 - TC_SARC

// Sub-Register 0x2A:03 - TC_SARL

// Sub-Register 0x2A:06 - TC_SARW

// Sub-Register 0x2A:08 - TC_PG_CTRL

// Sub-Register 0x2A:09 - TC_PG_STATUS

// Sub-Register 0x2A:0B - TC_PGDELAY, Transmitter Calibration – Pulse Generator Delay
union DW1000_SUB_REG_TC_PGDELAY
{
    uint8_t value;
};

// Sub-Register 0x2A:0C - TC_PGTEST

// Register file: 0x2A - Transmitter Calibration block
union DW1000_REG_TX_CAL
{

};

/**
 * Sub-Register 0x2B:00 - FS_RES1
 *
 * Register file: 0x2B - Frequency synthesiser control block, sub-register 0x00
 * is a reserved register. Please take care not to write to this area as doing
 * so may cause the DW1000 to malfunction.
 */
union DW1000_SUB_REG_FS_RES1
{
    /**
     * Byte[6:0] Frequency synthesiser - Reserved area 1. Please take care not
     * to write to this area as doing so may cause the DW1000 to malfunction.
     */
    uint8_t value[7];
};

// Sub-Register 0x2B:07 - FS_PLLCFG
union DW1000_SUB_REG_FS_PLLCFG
{
    uint8_t value[4];                   // Byte[3:0] Frequency synthesiser - PLL configuration
};

// Sub-Register 0x2B:0B - FS_PLLTUNE
union DW1000_SUB_REG_FS_PLLTUNE
{
    uint8_t value;                      // Bit[7:0] Frequency synthesiser - PLL Tuning
};

/**
 * Sub-Register 0x2B:0C - FS_RES2
 *
 * Register file: 0x2B - Frequency synthesiser control block, sub-register 0x0C
 * is a reserved area. Please take care not to write to this area as doing so
 * may cause the DW1000 to malfunction.
 */
union DW1000_SUB_REG_FS_RES2
{
    /**
     * Byte[1:0] Frequency synthesiser - Reserved area 2. Please take care not
     * to write to this area as doing so may cause the DW1000 to malfunction.
     */
    uint8_t rsvd[2];
};

// Sub-Register 0x2B:0E - FS_XTALT
union DW1000_SUB_REG_FS_XTALT
{
    struct
    {
        uint8_t fx_xtalt : 5;           // Bit[7:0] Frequency synthesiser - Crystal trim
        /**
         * Bit[7:5] Reserved.
         *
         * N.B.: Bits 7:5 must always be set to binary “011”. Failure to
         * maintain this value will result in DW1000 malfunction. Any change in
         * the value of this field will cause the DW1000 to malfunction.
         */
        uint8_t rsvd     : 3;
    };
    uint8_t value;
};

/**
 * Sub-Register 0x2B:0F - FS_RES3
 *
 * Register file: 0x2B - Frequency synthesiser control block, sub-register 0x0F
 * is a reserved area. Please take care not to write to this area as doing so
 * may cause the DW1000 to malfunction.
 */
union DW1000_SUB_REG_FS_RES3
{
    /**
     * Byte[5:0] Frequency synthesiser - Reserved area 3. Please take care not
     * to write to this area as doing so may cause the DW1000 to malfunction.
     */
    uint8_t rsvd[6];
};

// Register file: 0x2B - Frequency synthesiser control block
union DW1000_REG_FS_CTRL
{
    struct
    {
        /**
         * Byte[6:0] Sub-Register 0x2B:00 - FS_RES1
         *
         * Reserved.
         *
         * Please take care not to write to this area as doing so may cause the
         * DW1000 to malfunction.
         */
        union DW1000_SUB_REG_FS_RES1 fs_res1;
        /**
         * Byte[10:7] Sub-Register 0x2B:07 - FS_PLLCFG
         *
         * Frequency synthesiser - PLL configuration
         */
        union DW1000_SUB_REG_FS_PLLCFG fs_pllcfg;
        /**
         * Byte[11] Sub-Register 0x2B:0B - FS_PLLTUNE
         *
         * Frequency synthesiser - PLL Tuning
         */
        union DW1000_SUB_REG_FS_PLLTUNE fs_plltune;
        /**
         * Byte[13:12] Sub-Register 0x2B:0C - FS_RES2
         *
         * Reserved.
         *
         * Please take care not to write to this area as doing so may cause the
         * DW1000 to malfunction.
         */
        union DW1000_SUB_REG_FS_RES2 fs_res2;
        /**
         * Byte[14] Sub-Register 0x2B:0E - FS_XTALT
         *
         * Frequency synthesiser - Crystal trim
         */
        union DW1000_SUB_REG_FS_XTALT fs_xtalt;
        /**
         * Byte[20:15] Sub-Register 0x2B:0F - FS_RES3
         *
         * Reserved.
         *
         * Please take care not to write to this area as doing so may cause the
         * DW1000 to malfunction.
         */
        union DW1000_SUB_REG_FS_RES3 fs_res3;
    };
};

// Sub-Register 0x2C:00 - AON_WCFG
union DW1000_SUB_REG_AON_WCFG
{
    struct
    {
        /**
         * Bit[0] On Wake-up Run the (temperature and voltage) Analog-to-Digital
         * Convertors.
         */
        uint16_t onw_rad  : 1;
        uint16_t onw_rx   : 1;  // Bit[1] On Wake-up turn on the Receiver.
        uint16_t rsvd1    : 1;  // Bit[2] Reserved.
        /**
         * Bit[3] On Wake-up load the EUI from OTP memory into Register file:
         * 0x01 - Extended Unique Identifier.
         */
        uint16_t onw_leui : 1;
        uint16_t rsvd2    : 2;  // Bit[5:4] Reserved.
        /**
         * Bit[6] On Wake-upload configurations from the AON memory into the
         * host interface register set.
         */
        uint16_t onw_ldc  : 1;
        uint16_t onw_l64p : 1;  // Bit[7] On Wake-up load the Length64 receiver operating parameter set.
        uint16_t pres_slee : 1; // Bit[8] Preserve Sleep.
        uint16_t rsvd3    : 2;  // Bit[10:9] Reserved.
        uint16_t onw_llde : 1;  // Bit[11] On Wake-up load the LDE microcode.
        uint16_t onw_lld  : 1;  // Bit[12] On Wake-up load the LDOTUNE value from OTP.
        uint16_t rsvd4    : 3;  // Bit[15:13] Reserved.
    };
    // uint16_t value;
};

// Sub-Register 0x2C:02 - AON_CTRL
union DW1000_SUB_REG_AON_CTRL
{
    uint8_t value;
};

// Sub-Register 0x2C:03 - AON_RDAT
union DW1000_SUB_REG_AON_RDAT
{
    uint8_t value;
};

// Sub-Register 0x2C:04 - AON_ADDR
union DW1000_SUB_REG_AON_ADDR
{
    uint8_t value;
};

// Sub-Register 0x2C:05 - AON_RES1
union dw1000_syb_reg_aon_res1
{
    uint8_t value;                      // Bit[7:0] AON Reserved area 1.
};

// Sub-Register 0x2C:06 - AON_CFG0
union DW1000_SUB_REG_AON_CFG0
{
    struct
    {
        uint32_t sleep_en  : 1;         // Bit[0] Sleep Enable.
        uint32_t wake_pin  : 1;         // Bit[1] Wake using WAKEUP pin.
        uint32_t wake_spi  : 1;         // Bit[2] Wake using SPI access.
        uint32_t wake_cnt  : 1;         // Bit[3] Wake when sleep counter elapses.
        uint32_t lpdiv_en  : 1;         // Bit[4] Low power divider enable configuration.
        uint32_t lpclkdiva : 11;        // Bit[15:5] divider count for dividing the raw DW1000 XTAL oscillator frequency.
        uint32_t sleep_tim : 16;        // Bit[31:16] Sleep time.
    };
    // uint32_t value;
};

// Sub-Register 0x2C:0A - AON_CFG1
union DW1000_SUB_REG_AON_CFG1
{
    struct
    {
        uint16_t sleep_ce : 1;
        uint16_t smxx     : 1;
        uint16_t lposc_c  : 1;
        uint16_t rsvd     : 13;
    };
};

// Register file: 0x2C - Always-on system control interface
union DW1000_REG_AON
{
    struct
    {
        union DW1000_SUB_REG_AON_WCFG aon_wcfg;
        union DW1000_SUB_REG_AON_CTRL aon_ctrl;
        union DW1000_SUB_REG_AON_RDAT aon_rdat;
        union DW1000_SUB_REG_AON_ADDR aon_addr;
        union dw1000_syb_reg_aon_res1 aon_res1;
        union DW1000_SUB_REG_AON_CFG0 aon_cfg0;
        union DW1000_SUB_REG_AON_CFG1 aon_cfg1;
    };
};

_Static_assert(sizeof(union DW1000_REG_AON) == 12, "union DW1000_REG_AON must be 12 bytes");

// Sub-Register 0x2D:00 - OTP_WDAT
union DW1000_SUB_REG_OTP_WDAT
{
    /**
     * Bit[31:0] Register file: 0x2D - OTP Memory Interface, sub-register 0x00
     * is a 32-bit register. The data value to be programmed into an OTP
     * location should be written here before invoking the programming function.
     */
    uint32_t value;
};

// Sub-Register 0x2D:04 - OTP_ADDR
union DW1000_SUB_REG_OTP_ADDR
{
    struct
    {
        /**
         * Bit[10:0] This 11-bit field specifies the address within OTP memory
         * that will be accessed read or written.
         */
        uint16_t otpaddr : 11;
        uint16_t rsvd    : 5;           // Bit[15:11] Reserved.
    };
    // uint16_t value;
};

// Sub-Register 0x2D:06 - OTP_CTRL
union DW1000_SUB_REG_OTP_CTRL
{
    struct
    {
        uint16_t  otprden : 1;          // Bit[0] This bit forces the OTP into manual read mode.
        /**
         * Bit[1] This bit commands a read operation from the address specified
         * in the OTP_ADDR register the value read will then be available in
         * the OTP_RDAT register.
         */
        uint16_t  otpread : 1;
        uint16_t  rsvd1   : 1;          // Bit[2] Reserved.
        uint16_t  otpmrwr : 1;          // Bit[3] OTP mode register write.
        uint16_t  rsvd2   : 2;          // Bit[5:4] Reserved.
        /**
         * Bit[6] Setting this bit will cause the contents of OTP_WDAT to be
         * written to OTP_ADDR.
         */
        uint16_t  otpprog : 1;
        uint16_t  otpmr   : 4;          // Bit[10:7] OTP Mode register.
        uint16_t  rsvd3   : 4;          // Bit[14:11] Reserved.
        uint16_t  ldeload : 1;          // Bit[15] This bit forces a load of LDE microcode.
    };
    // uint16_t value;
};

// Sub-Register 0x2D:08 - OTP_STAT
union DW1000_SUB_REG_OTP_STAT
{
    struct
    {
        uint16_t otpprgd : 1;           // Bit[0] OTP Programming Done.
        uint16_t otpvpok : 1;           // Bit[1] OTP Programming Voltage OK.
        uint16_t rsvd    : 14;          // Bit[15:2] Reserved.
    };
    // uint16_t value;
};

// Sub-Register 0x2D:0A - OTP_RDAT
union DW1000_SUB_REG_OTP_RDAT
{
    uint32_t value;                     // Bit[31:0] R, OTP Read Data
};

// Sub-Register 0x2D:0E - OTP_SRDAT
union DW1000_SUB_REG_OTP_SRDAT
{
    uint32_t value;                     // Bit[31:0] OTP Special Register Read Data
};

// Sub-Register 0x2D:12 - OTP_SF
union DW1000_SUB_REG_OTP_SF
{
    struct
    {
        /**
         * Bit[0] This bit when set initiates a load of the operating parameter
         * set selected by the OPS_SEL configuration below. (This control is in
         * the OTP block because the parameter sets are in OTP memory during
         * their development, only moving to ROM for the production IC).
         */
        uint8_t ops_kick : 1;
        /**
         * Bit[1] This bit when set initiates the loading of the LDOTUNE_CAL
         * parameter from OTP address 0x4 into the register Sub-Register 0x28:30
         * - LDOTUNE. See the section Waking from sleep for more details.
         */
        uint8_t ldo_kick : 1;
        uint8_t rsvd1    : 3;           // Bit[4:2] Reserved.
        uint8_t ops_sel  : 2;           // Bit[6:5] Operating parameter set selection.
        uint8_t rsvd2    : 1;           // Bit[7] Reserved.
    };
    // uint8_t value;
};

// Register file: 0x2D - OTP Memory Interface
union DW1000_REG_OTP_IF
{
    struct
    {
        union DW1000_SUB_REG_OTP_WDAT otp_wdat;
        union DW1000_SUB_REG_OTP_ADDR otp_addr;
        union DW1000_SUB_REG_OTP_CTRL otp_ctrl;
        union DW1000_SUB_REG_OTP_STAT otp_stat;
        union DW1000_SUB_REG_OTP_RDAT otp_rdat;
        union DW1000_SUB_REG_OTP_SRDAT otp_srdat;
        union DW1000_SUB_REG_OTP_SF otp_sf;
    };
};

_Static_assert(sizeof(union DW1000_REG_OTP_IF) == 19, "union DW1000_REG_OTP_IF must be 19 bytes");

// Sub-Register 0x2E:0000 - LDE_THRESH, LDE Threshold report
union DW1000_SUB_REG_LDE_THRESH
{
    uint16_t value;                     // LDE Threshold report
};

_Static_assert(sizeof(union DW1000_SUB_REG_LDE_THRESH) == 2, "union DW1000_SUB_REG_LDE_THRESH must be 2 bytes");

// Sub-Register 0x2E:0806 - LDE_CFG1, LDE Configuration Register 1
union DW1000_SUB_REG_LDE_CFG1
{
    struct
    {
        uint8_t ntm   : 5;              // Bit[4:0] Noise Threshold Multiplier.
        uint8_t pmult : 3;              // Bit[7:5] Peak Multiplier.
    };
    uint8_t value;                      // LDE Configuration Register 1
};

_Static_assert(sizeof(union DW1000_SUB_REG_LDE_CFG1) == 1, "union DW1000_SUB_REG_LDE_CFG1 must be 1 bytes");

// Sub-Register 0x2E:1000 - LDE_PPINDX, LDE Peak Path Index
union DW1000_SUB_REG_LDE_PPINDX
{
    uint16_t value;
};

// Sub-Register 0x2E:1002 - LDE_PPAMPL, LDE Peak Path Amplitude
union DW1000_SUB_REG_LDE_PPAMPL
{
    uint16_t value;
};

// Sub-Register 0x2E:1804 - LDE_RXANTD, LDE Receive Antenna Delay configuration
union DW1000_SUB_REG_LDE_RXANTD
{
    uint16_t value;
};

// Sub-Register 0x2E:1806 - LDE_CFG2, LDE Configuration Register 2
union DW1000_SUB_REG_LDE_CFG2
{
    uint16_t value;                     // LDE Configuration Register 2
};

_Static_assert(sizeof(union DW1000_SUB_REG_LDE_CFG2) == 2, "union DW1000_SUB_REG_LDE_CFG2 must be 2 bytes");

// Sub-Register 0x2E:2804 - LDE_REPC, LDE Replica Coefficient configuration
union DW1000_SUB_REG_LDE_REPC
{
    uint16_t value;
};

/**
 * Register file: 0x2E - Leading Edge Detection Interface
 *
 * PLEASE NOTE: Other areas within the address space of Register file: 0x2E –
 * Leading Edge Detection Interface are reserved. To ensure proper operation of
 * the LDE algorithm (i.e. to avoid loss of performance or a malfunction), care
 * must be taken not to write to any byte locations other than those defined in
 * the sub-sections.
 */
union DW1000_REG_LDE_IF
{

};

_Static_assert(sizeof(union DW1000_REG_LDE_IF) == 0, "union DW1000_REG_LDE_IF must be 0 bytes");

// Sub-Register 0x2F:00 - Event Counter Control
union DW1000_SUB_REG_EVC_CTRL
{
    struct
    {
        uint32_t evc_en  : 1;           // Event Counters Enable. SRW
        uint32_t evc_clr : 1;           // Event Counters Clear. SRW
        uint32_t rsvd    : 30;          // Reserved
    };
    uint32_t value;
};

// Sub-Register 0x2F:04 - PHR Error Counter
union DW1000_SUB_REG_EVC_PHE
{
    struct
    {
        uint16_t evc_phe : 12;          // PHR Error Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:06 - RSD Error Counter
union DW1000_SUB_REG_EVC_RSE
{
    struct
    {
        uint16_t evc_rse : 12;          // Reed Solomon decoder (Frame Sync Loss) Error Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:08 - FCS Good Counter
union DW1000_SUB_REG_EVC_FCG
{
    struct
    {
        uint16_t evc_fcg : 12;          // Frame Check Sequence Good Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:0A - FCS Error Counter
union DW1000_SUB_REG_EVC_FCE
{
    struct
    {
        uint16_t evc_fce : 12;          // Frame Check Sequence Error Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:0C - Frame Filter Rejection Counter
union DW1000_SUB_REG_EVC_FFR
{
    struct
    {
        uint16_t evc_ffr : 12;          // Frame Filter Rejection Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:0E - RX Overrun Error Counter
union DW1000_SUB_REG_EVC_OVR
{
    struct
    {
        uint16_t evc_ovr : 12;          // RX Overrun Error Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:10 - SFD Timeout Error Counter
union DW1000_SUB_REG_EVC_STO
{
    struct
    {
        uint16_t evc_sto : 12;          // SFD (start of frame delimiter) timeout errors Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:12 - Preamble Detection Timeout Event Counter
union DW1000_SUB_REG_EVC_PTO
{
    struct
    {
        uint16_t evc_pto : 12;          // Preamble Detection Timeout Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:14 - RX Frame Wait Timeout Event Counter
union DW1000_SUB_REG_EVC_FWTO
{
    struct
    {
        uint16_t evc_fwto : 12;         // RX Frame Wait Timeout Event Counter. RO
        uint16_t rsvd     : 4;          // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:16 - TX Frame Sent Counter
union DW1000_SUB_REG_EVC_TXFS
{
    struct
    {
        uint16_t evc_txfs : 12;         // TX Frame Sent Event Counter. RO
        uint16_t rsvd     : 4;          // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:18 - Half Period Warning Counter
union DW1000_SUB_REG_EVC_HPW
{
    struct
    {
        uint16_t rvc_hpw : 12;          // Half Period Warning Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
};

// Sub-Register 0x2F:1A - Transmitter Power-Up Warning Counter
union DW1000_SUB_REG_EVC_TPW
{
    struct
    {
        uint16_t evc_tpw : 12;          // Transmitter Power-Up Warning Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:1C - Digital Diagnostics Reserved Area 1
union DW1000_SUB_REG_EVC_RES1
{
    struct
    {
        uint64_t evc_res1;              // Digital Diagnostics Reserved Area 1. RW
    };
    uint64_t value;
};

// Sub-Register 0x2F:24 - Test Mode Control Register
union DW1000_SUB_REG_DIAG_TMC
{
    struct
    {
        uint16_t ravd1   : 4;           // Reserved 1
        uint16_t tx_pstm : 1;           // Transmit Power Spectrum Test Mode.
        uint16_t rsvd2   : 11;          // Reserved 2
    };
    uint16_t value : 16;
};

// Register file: 0x2F - Digital Diagnostics Interface
union DW1000_REG_DIG_DIAG
{
    struct
    {
        union DW1000_SUB_REG_EVC_CTRL evc_ctrl; // Sub-Register 0x2F:00 - Event Counter Control
        union DW1000_SUB_REG_EVC_PHE evc_phe;   // Sub-Register 0x2F:04 - PHR Error Counter
        union DW1000_SUB_REG_EVC_RSE evc_rse;   // Sub-Register 0x2F:06 - RSD Error Counter
        union DW1000_SUB_REG_EVC_FCG evc_fcg;   // Sub-Register 0x2F:08 - FCS Good Counter
        union DW1000_SUB_REG_EVC_FCE evc_fce;   // Sub-Register 0x2F:0A - FCS Error Counter
        union DW1000_SUB_REG_EVC_FFR evc_ffr;   // Sub-Register 0x2F:0C - Frame Filter Rejection Counter
        union DW1000_SUB_REG_EVC_OVR evc_ovr;   // Sub-Register 0x2F:0E - RX Overrun Error Counter
        union DW1000_SUB_REG_EVC_STO evc_sto;   // Sub-Register 0x2F:10 - SFD Timeout Error Counter
        union DW1000_SUB_REG_EVC_PTO evc_pto;   // Sub-Register 0x2F:12 - Preamble Detection Timeout Event Counter
        union DW1000_SUB_REG_EVC_FWTO evc_fwto; // Sub-Register 0x2F:14 - RX Frame Wait Timeout Event Counter
        union DW1000_SUB_REG_EVC_TXFS evc_txfs; // Sub-Register 0x2F:16 - TX Frame Sent Counter
        union DW1000_SUB_REG_EVC_HPW evc_hpw;   // Sub-Register 0x2F:18 - Half Period Warning Counter
        union DW1000_SUB_REG_EVC_TPW evc_tpw;   // Sub-Register 0x2F:1A - Transmitter Power-Up Warning Counter
        union DW1000_SUB_REG_EVC_RES1 evc_res1; // Sub-Register 0x2F:1C - EVC_RES1
        union DW1000_SUB_REG_DIAG_TMC diag_tmc; // Sub-Register 0x2F:24 - Digital Diagnostics Test Mode Control
        uint8_t rsvd[3];
    };
    uint8_t value[41];
};

// Register files: 0x30 to 0x35 - Reserved

// Sub-Register 0x36:00 - PMSC_CTRL0, PMSC Control Register 0
union DW1000_SUB_REG_PMSC_CTRL0
{
    struct
    {
        uint32_t sysclks     : 2; // Bit[1:0] System Clock Selection.
        uint32_t rxclks      : 2; // Bit[3:2] Receiver Clock Selection.
        uint32_t txclks      : 2; // Bit[5:4] Transmitter Clock Selection.
        uint32_t face        : 1; // Bit[6] Force Accumulator Clock Enable.
        uint32_t rsvd1       : 3; // Bit[9:7] Reserved.
        uint32_t adcce       : 1; // Bit[10] (temperature and voltage) Analog-to-Digital Convertor Clock Enable.
        uint32_t rsvd2       : 4; // Bit[14:11] Reserved.
        uint32_t amce        : 1; // Bit[15] Accumulator Memory Clock Enable.
        uint32_t gpce        : 1; // Bit[16] GPIO clock Enable.
        uint32_t gprn        : 1; // Bit[17] GPIO reset (NOT), active low.
        uint32_t gpdce       : 1; // Bit[18] GPIO De-bounce Clock Enable.
        uint32_t gpdrn       : 1; // Bit[19] GPIO de-bounce reset (NOT), active low.
        uint32_t rsvd3       : 3; // Bit[22:20] Reserved.
        uint32_t khzclken    : 1; // Bit[23] Kilohertz clock Enable.
        /**
         * Bit[24] Value 0 means normal (TX sequencing control), value 1 means
         * RX SNIFF mode control.
         */
        uint32_t pll2_seq_en : 1;
        uint32_t rsvd4       : 3; // Bit[27:25] Reserved.
        /**
         * Bit[31:28] These four bits reset the IC TX, RX, Host Interface and
         * the PMSC itself, essentially allowing a reset of the IC under
         * software control.
         */
        uint32_t softreset   : 4;
    };
    struct
    {
        uint16_t word_l;
        uint16_t word_h;
    };
    uint32_t value;
};

_Static_assert(sizeof(union DW1000_SUB_REG_PMSC_CTRL0) == 4, "union DW1000_SUB_REG_PMSC_CTRL0 must be 4 bytes");

// Sub-Register 0x36:04 - PMSC_CTRL1, PMSC Control Register 1
union DW1000_SUB_REG_PMSC_CTRL1
{
    struct
    {
        uint32_t rsvd1     : 1;         // Bit[0] Reserved.
        uint32_t arx2int   : 1;         // Bit[1] Automatic transition from receive mode into the INIT state.
        uint32_t rsvd2     : 1;         // Bit[2] Reserved.
        uint32_t pktseq    : 8;         // Bit[10:3] Writing 0 to PKTSEQ disables PMSC control of analog RF subsystems.
        uint32_t atxslp    : 1;         // Bit[11] After TX automatically Sleep.
        uint32_t arxslp    : 1;         // Bit[12] After RX automatically Sleep.
        uint32_t snoze     : 1;         // Bit[13] Snooze Enable.
        uint32_t snozr     : 1;         // Bit[14] Snooze Repeat.
        uint32_t pllsyn    : 1;         // Bit[15] This enables a special 1 GHz clock used for some external SYNC modes.
        uint32_t rsvd3     : 1;         // Bit[16] Reserved.
        uint32_t lderune   : 1;         // Bit[17] LDE run enable.
        uint32_t rsvd4     : 8;         // Bit[25:18] Reserved.
        uint32_t khzclkdiv : 6;         // Bit[31:26] Kilohertz clock divisor.
    };
    struct
    {
        uint16_t word_l;
        uint16_t word_h;
    };
    uint32_t value;
};

_Static_assert(sizeof(union DW1000_SUB_REG_PMSC_CTRL1) == 4, "union DW1000_SUB_REG_PMSC_CTRL1 must be 4 bytes");

// Sub-Register 0x36:08 - PMSC_RES1, PMSC reserved area 1
union DW1000_SUB_REG_PMSC_RES1
{
    uint32_t value;
};

// Sub-Register 0x36:0C - PMSC_SNOZT, PMSC Snooze Time Register
union DW1000_SUB_REG_PMSC_SNOZT
{
    uint32_t value;
};

// Sub-Register 0x36:10 - PMSC_RES2, PMSC reserved area 2
union DW1000_SUB_REG_PMSC_RES2
{
    uint8_t value[22];
};

// Sub-Register 0x36:26 - PMSC_TXFSEQ, PMSC fine grain TX sequencing control
union DW1000_SUB_REG_PMSC_TXFSEQ
{
    uint16_t value;
};

// Sub-Register 0x36:28 - PMSC_LEDC, PMSC LED Control Register
union DW1000_SUB_REG_PMSC_LEDC
{
    uint32_t value;
};

// Register file: 0x36 - Power Management and System Control
union DW1000_REG_PMSC
{
    struct
    {
        union DW1000_SUB_REG_PMSC_CTRL0 pmsc_ctrl0;
        union DW1000_SUB_REG_PMSC_CTRL1 pmsc_ctrl1;
        union DW1000_SUB_REG_PMSC_RES1 pmsc_res1;
        union DW1000_SUB_REG_PMSC_SNOZT pmsc_snozt;
        union DW1000_SUB_REG_PMSC_RES2 pmsc_res2;
        union DW1000_SUB_REG_PMSC_TXFSEQ pmsc_txfseq;
        union DW1000_SUB_REG_PMSC_LEDC pmsc_ledc;
    };
};

_Static_assert(sizeof(union DW1000_REG_PMSC) == 44, "union DW1000_REG_PMSC must be 44 bytes");

// Register files: 0x37 to 0x3F - Reserved

// Single octet header of the non-indexed SPI transaction
union dw1000_tran_header1
{
    struct
    {
        uint8_t rid : 6;                // Register file ID - Range 0x00 to 0x3F (64 locations)
        uint8_t si  : 1;                // Bit = 0, says sub-index is not present
        uint8_t op  : 1;                // Operation: 0 = Read, 1 = Write
    };
    uint8_t value;
};

// Two octet header of the short indexed SPI transaction
union dw1000_tran_header2
{
    struct
    {
        uint16_t rid      : 6;          // Register file ID - Range 0x00 to 0x3F (64 locations)
        uint16_t si       : 1;          // Bit = 1, says sub-index is not present
        uint16_t op       : 1;          // Operation: 0 = Read, 1 = Write
        uint16_t sub_addr : 7;          // 7-bit Register File sub-address, range 0x00 to 0x7F (128 byte locations)
        uint16_t ext      : 1;          // Extended Address: 0 = no
    };
    uint8_t value[2];
};

// Three octet header of the long indexed SPI transaction
union dw1000_tran_header3
{
    struct
    {
        uint8_t rid         : 6;        // Register file ID - Range 0x00 to 0x3F (64 locations)
        uint8_t si          : 1;        // Bit = 1, says sub-index is not present
        uint8_t op          : 1;        // Operation: 0 = Read, 1 = Write
        uint16_t sub_addr_l : 7;        // Low order 7 bits of 15-bit Register file sub-address, range 0x0000 to 0x7FFF (32768 byte locations)
        uint16_t ext        : 1;        // Extended Address: 1 = yes
        uint16_t sub_addr_h : 8;        // High order 8 bits of 15-bit Register file sub-address, range 0x0000 to 0x7FFF (32768 byte locations)
    };
    uint8_t value[3];
};


//! IEEE 802.15.4e standard blink. It is a 12-byte frame composed of the following fields.
union ieee_blink_frame_t
{
//! Structure of IEEE blink frame
    struct
    {
        uint8_t fctrl;              //!< Frame type (0xC5 for a blink) using 64-bit addressing
        uint8_t seq_num;            //!< Sequence number, incremented for each new frame
        union
        {
            uint64_t long_address;  //!< Device ID TODOs::depreciated nomenclature
            uint64_t euid;          //!< extended unique identifier
        };
    };
    // uint8_t array[10]; //!< Array of size blink frame
};

_Static_assert(sizeof(union ieee_blink_frame_t) == 10, "union ieee_blink_frame_t must be 10 bytes");

/**
 * This frame type field is a 3-bit field that indicates the type of frame.
 * Below lists the eight possible frame types and their assignment in IEEE
 * 802.15.4-2011.
 */
enum ieee_802_15_4_frame_type
{
    IEEE_802_15_4_FTYPE_BEACON = 0,
    IEEE_802_15_4_FTYPE_DATA = 1,
    IEEE_802_15_4_FTYPE_ACK,
    IEEE_802_15_4_FTYPE_MAC_CMD,
};

/**
 * The destination addressing mode field (2-bits) specifies whether the frame
 * contains a destination address and if so the size of the address field.
 */
enum ieee_802_15_4_dst_addr_mode
{
    IEEE_802_15_4_DAM_NO_ADDR = 0,
    IEEE_802_15_4_DAM_SHORT_ADDR = 1,
    IEEE_802_15_4_DAM_RSVD,
    IEEE_802_15_4_DAM_EXT_ADDR,
};

/**
 * The source addressing mode field (2-bits) specifies whether the frame
 * contains a source address and if so the size of the address field.
 */
enum ieee_802_15_4_src_addr_mode
{
    IEEE_802_15_4_SAM_NO_ADDR = 0,
    IEEE_802_15_4_SAM_SHORT_ADDR = 1,
    IEEE_802_15_4_SAM_RSVD,
    IEEE_802_15_4_SAM_EXT_ADDR,
};

/**
 * The frame control field in the MAC header
 */
union ieee_802_15_4_mac_fctrl_t
{
    struct
    {
        uint16_t ftype   : 3;           // Bit[2:0] Frame Type.
        uint16_t se      : 1;           // Bit[3] Security Enabled.
        uint16_t fpend   : 1;           // Bit[4] Frame Pending.
        uint16_t ack_req : 1;           // Bit[5] ACK Request .
        uint16_t pan_id  : 1;           // Bit[6] PAN ID Compress.
        uint16_t rsvd    : 3;           // Bit[9:7] Reserved.
        uint16_t dam     : 2;           // Bit[11:10] Dest. Address Mode.
        uint16_t fver    : 2;           // Bit[13:12] Frame Version.
        uint16_t sam     : 2;           // Bit[15:14] Source Address Mode.
    };
    uint16_t value;
};

//! IEEE 802.15.4 standard ranging frames
union ieee_rng_request_frame_t
{
//! Structure of range request frame
    struct
    {
        uint16_t fctrl;                 //!< Frame control (0x8841 to indicate a data frame using 16-bit addressing)
        uint8_t seq_num;                //!< Sequence number, incremented for each new frame
        uint16_t pan_id;                //!< pan_id
        uint16_t dst_addr;              //!< Destination address
        uint16_t src_addr;              //!< Source address
        uint8_t code;                   //!< Response code for the request
    };
    // uint8_t array[10];  //!< Array of size range request frame
};

_Static_assert(sizeof(union ieee_rng_request_frame_t) == 10, "union ieee_rng_request_frame_t must be 10 bytes");

union dw1000_rng_init_msg_t
{
//! Structure of range request frame
    struct
    {
        uint16_t fctrl;                 //!< Frame control (0x8841 to indicate a data frame using 16-bit addressing)
        uint8_t seq_num;                //!< Sequence number, incremented for each new frame
        uint16_t pan_id;                //!< pan_id
        uint16_t dst_addr;              //!< Destination address
        uint16_t src_addr;              //!< Source address
        uint8_t code;                   //!< Function code (0x20 to indicate the ranging init message)
        uint16_t tag_addr;
        uint16_t resp_delay;
    };
};

union dw1000_poll_msg_t
{
//! Structure of range request frame
    struct
    {
        uint16_t fctrl;                 //!< Frame control (0x8841 to indicate a data frame using 16-bit addressing)
        uint8_t seq_num;                //!< Sequence number, incremented for each new frame
        uint16_t pan_id;                //!< pan_id
        uint16_t dst_addr;              //!< Destination address
        uint16_t src_addr;              //!< Source address
        uint8_t code;                   //!< Function code (0x61 to indicate the poll message)
    };
};

union dw1000_resp_msg_t
{
//! Structure of range request frame
    struct
    {
        uint16_t fctrl;                 //!< Frame control (0x8841 to indicate a data frame using 16-bit addressing)
        uint8_t seq_num;                //!< Sequence number, incremented for each new frame
        uint16_t pan_id;                //!< pan_id
        uint16_t dst_addr;              //!< Destination address
        uint16_t src_addr;              //!< Source address
        uint8_t code;                   //!< Function code (0x50 to indicate the poll message)
        uint32_t tof;                   //!< Calculated Time-of-Flight
    };
};

union dw1000_final_msg_t
{
//! Structure of range request frame
    struct
    {
        uint16_t fctrl;                 //!< Frame control (0x8841 to indicate a data frame using 16-bit addressing)
        uint8_t seq_num;                //!< Sequence number, incremented for each new frame
        uint16_t pan_id;                //!< pan_id
        uint16_t dst_addr;              //!< Destination address
        uint16_t src_addr;              //!< Source address
        uint8_t code;                   //!< Function code (0x69 to indicate the poll message)
        uint32_t t1;                    //!< Resp RX time – Poll TX time
        uint32_t t2;                    //!< Final TX time – Resp RX time
    };
};

#pragma pop

struct dw1000_reg
{
    const char *mnemonic;
    const char *desc;
    uint16_t length;
    uint16_t reg_file_id;
    uint8_t reg_file_type;
};

struct dw1000_context
{
    uint8_t tx_buf[64] __attribute__((aligned(4)));
    uint8_t rx_buf[64] __attribute__((aligned(4)));
    struct spi_config spi_cfg;
    struct gpio_config gpio_irq_cfg;
    struct gpio_config gpio_rst_cfg;
    // struct gpio_config;
    /**
     * System Configuration
     */
    union DW1000_REG_SYS_CFG sys_cfg;
    union DW1000_REG_RX_FWTO rx_fwto;
    union DW1000_REG_RX_SNIFF rx_sniff;
    union DW1000_SUB_REG_GPIO_MODE gpio_mode;
    union DW1000_REG_AON aon;
    union DW1000_REG_OTP_IF otp_if;
    union DW1000_REG_PMSC pmsc;
    union DW1000_REG_SYS_STATUS sys_status;
    union DW1000_REG_SYS_MASK sys_mask;
    /**
     * Channel Configuration
     */
    union DW1000_REG_CHAN_CTRL chan_ctrl;
    union DW1000_REG_FS_CTRL fs_ctrl;
    /**
     * Transmitter Configuration
     */
    union DW1000_REG_TX_FCTRL tx_fctrl;
    union DW1000_REG_TX_POWER tx_power;
    /**
     * Receiver Configuration
     */
    union DW1000_REG_DRX_CONF drx_conf;
    union DW1000_REG_RF_CONF rf_conf;
    /**
     * Default Configurations that should be modified
     */
    union DW1000_REG_AGC_CTRL agc_ctrl;
    union DW1000_SUB_REG_LDE_CFG1 lde_cfg1;
    union DW1000_SUB_REG_LDE_CFG2 lde_cfg2;
    union DW1000_SUB_REG_LDE_REPC lde_repc;
    union DW1000_SUB_REG_TC_PGDELAY tc_pgdelay;
    union DW1000_SUB_REG_PMSC_CTRL0 pmsc_ctrl0;
    union DW1000_SUB_REG_EC_CTRL ec_ctrl;
    //
    enum dw1000_ads_state ads_twr_state;
    uint16_t tar_addr;
    uint16_t my_addr;
    uint8_t seq_num;
    bool is_standard_sfd;
    bool is_txprf_16mhz;
    bool lde_run_enable;
    bool sleep_enable;
};

void dw1000_ctx_init();
void dw1000_unit_test();

#endif  // ~ DW1000_H
