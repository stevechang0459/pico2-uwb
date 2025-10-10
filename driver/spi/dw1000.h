#ifndef DW1000_H
#define DW1000_H

#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "spi.h"

/**
 * The chipping rate given by the IEEE 802.15.4-2011 standard [1] is 499.2 MHz.
 * DW1000 system clocks are referenced to this frequency.
 */
#define IEEE_802_15_4_2001_CHIPPING_RATE (499200000ULL)

/**
 * A 63.8976 GHz sampling clock is associated with ranging for the IEEE 802.15.4-2011 standard,
 * where a 15.65 picosecond time period is referred to, it is an approximation to the period of this clock.
 *
 * 63.8976 GHz = 128 * 499.2 MHz
 *
 * approximately 15.65 picoseconds
 */
#define DW1000_SAMPLING_CLOCK           (128ULL * IEEE_802_15_4_2001_CHIPPING_RATE)

/**
 * The chipping rate given by the IEEE 802.15.4-2011 standard [1] is 499.2 MHz.
 * DW1000 system clocks are referenced to this frequency. Where the system clock
 * frequency is given as 125 MHz, this is an approximation to the actual system
 * clock frequency of 124.8 MHz.
 *
 * 124.8 MHz
 * = 499.2 MHz / 4 = IEEE_802_15_4_2001_CHIPPING_RATE / 4
 * = 63.8976 GHz / 512 = DW1000_SAMPLING_CLOCK / 512
 *
 * approximately 8 nanoseconds
 */
#define DW1000_SYS_CLOCK                (IEEE_802_15_4_2001_CHIPPING_RATE / 4ULL)

// #define DX_TIME_MS(t)                   ((t) * DW1000_SYS_CLOCK / 1000)
// #define DX_TIME_US(t)                   ((t) * DW1000_SYS_CLOCK / 1000000)
// #define DX_TIME_NS(t)                   ((t) > 8 ? (t) * DW1000_SYS_CLOCK / 1000000000 : 0)

#define DX_TIME_MS(t)                   ((uint64_t)(t) * DW1000_SAMPLING_CLOCK / 1000ULL)
#define DX_TIME_US(t)                   ((uint64_t)(t) * DW1000_SAMPLING_CLOCK / 1000000ULL)
#define DX_TIME_NS(t)                   ((uint64_t)(t) * DW1000_SAMPLING_CLOCK / 1000000000ULL)

// DW1000 Register File IDs
enum dw1000_reg_file_id
{
    DW1000_DEV_ID     = 0x00,
    DW1000_EUI        = 0x01,
    DW1000_PANADR     = 0x03,
    DW1000_SYS_CFG    = 0x04,
    DW1000_SYS_TIME   = 0x06,
    DW1000_TX_FCTRL   = 0x08,
    DW1000_TX_BUFFER  = 0x09,
    DW1000_DX_TIME    = 0x0a,
    DW1000_RX_FWTO    = 0x0c,
    DW1000_SYS_CTRL   = 0x0d,
    DW1000_SYS_MASK   = 0x0e,
    DW1000_SYS_STATUS = 0x0f,
    DW1000_RX_FINFO   = 0x10,
    DW1000_RX_BUFFER  = 0x11,
    DW1000_RX_FQUAL   = 0x12,
    DW1000_RX_TTCKI   = 0x13,
    DW1000_RX_TTCKO   = 0x14,
    DW1000_RX_TIME    = 0x15,
    DW1000_TX_TIME    = 0x17,
    DW1000_TX_ANTD    = 0x18,
    DW1000_SYS_STATE  = 0x19,
    DW1000_ACK_RESP_T = 0x1a,
    DW1000_RX_SNIFF   = 0x1d,
    DW1000_TX_POWER   = 0x1e,
    DW1000_CHAN_CTRL  = 0x1f,
    DW1000_USR_SFD    = 0x21,
    DW1000_AGC_CTRL   = 0x23,
    DW1000_EXT_SYNC   = 0x24,
    DW1000_ACC_MEM    = 0x25,
    DW1000_GPIO_CTRL  = 0x26,
    DW1000_DRX_CONF   = 0x27,
    DW1000_RF_CONF    = 0x28,
    DW1000_TX_CAL     = 0x2a,
    DW1000_FS_CTRL    = 0x2b,
    DW1000_AON        = 0x2c,
    DW1000_OTP_IF     = 0x2d,
    DW1000_LDE_CTRL   = 0x2e,
    DW1000_DIG_DIAG   = 0x2f,
    DW1000_PMSC       = 0x36,
    DW1000_REG_FILE_ID_MAX = 0x40
};

enum dw1000_sub_reg_ofs_dig_diag
{
    DW1000_EVC_CTRL = 0x00,
    DW1000_EVC_PHE  = 0x04,
    DW1000_EVC_RSE  = 0x06,
    DW1000_EVC_FCG  = 0x08,
    DW1000_EVC_FCE  = 0x0a,
    DW1000_EVC_FFR  = 0x0c,
    DW1000_EVC_OVR  = 0x0e,
    DW1000_EVC_STO  = 0x10,
    DW1000_EVC_PTO  = 0x12,
    DW1000_EVC_FWTO = 0x14,
    DW1000_EVC_TXFS = 0x16,
    DW1000_EVC_HPW  = 0x18,
    DW1000_EVC_TPW  = 0x1a,
    DW1000_EVC_RES1 = 0x1d,
    DW1000_EVC_TMC  = 0x24,
};

enum dw1000_sub_reg_ofs_aon
{
    DW1000_AON_WCFG = 0x00,
    DW1000_AON_CTRL = 0x02,
    DW1000_AON_RDAT = 0x03,
    DW1000_AON_ADDR = 0x04,
    DW1000_AON_CFG0 = 0x06,
    DW1000_AON_CFG1 = 0x0a,
};

enum dw1000_sub_reg_ofs_drx_conf
{
    DW1000_DRX_TUNE0b   = 0x02,
    DW1000_DRX_TUNE1a   = 0x04,
    DW1000_DRX_TUNE1b   = 0x06,
    DW1000_DRX_TUNE2    = 0x08,
    DW1000_DRX_SFDTOC   = 0x20,
    DW1000_DRX_PRETOC   = 0x24,
    DW1000_DRX_TUNE4H   = 0x26,
    DW1000_DRX_CAR_INT  = 0x28,
    DW1000_RXPACC_NOSAT = 0x2c,
};

enum dw1000_reg_file_type
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

enum drx_tune2_value
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

#pragma push
#pragma pack(1)

// Register file: 0x00 – Device Identifier
union dw1000_reg_dev_id
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

// Register file: 0x01 – Extended Unique Identifier
union dw1000_reg_eui
{
    uint8_t value[8];
};

// Register file: 0x02 – Reserved

// Register file: 0x03 – PAN Identifier and Short Address
union dw1000_reg_panadr
{
    struct
    {
        uint32_t short_addr : 16;
        uint32_t pan_id     : 16;
    };
    uint32_t value;
};

// Register file: 0x04 – System Configuration
union dw1000_reg_sys_cfg
{
    struct
    {
        uint32_t ffen       : 1;        // Frame Filtering Enable
        uint32_t ffbc       : 1;        // Frame Filtering Behave as a Coordinator
        uint32_t ffab       : 1;        // Frame Filtering Allow Beacon frame reception
        uint32_t ffad       : 1;        // Frame Filtering Allow Acknowledgment frame reception
        uint32_t ffaa       : 1;        // Frame Filtering Allow MAC command frame reception
        uint32_t ffam       : 1;        // Frame Filtering Allow MAC command frame reception
        uint32_t ffar       : 1;        // Frame Filtering Allow Reserved frame types
        uint32_t ffa4       : 1;        // Frame Filtering Allow frames with frame type field of 4
        //
        uint32_t ffa5       : 1;        // Frame Filtering Allow frames with frame type field of 5
        uint32_t hirq_pol   : 1;        // Host interrupt polarity
        uint32_t spi_edge   : 1;        // SPI data launch edge
        uint32_t dis_fce    : 1;        // Disable frame check error handling
        uint32_t dis_drxb   : 1;        // Disable Double RX Buffer
        uint32_t dis_phe    : 1;        // Disable receiver abort on PHR error
        uint32_t dis_rsde   : 1;        // Disable Receiver Abort on RSD error
        /**
         * This bit allows selection of the initial seed value for the FCS
         * generation and checking function that is set at the start of each
         * frame transmission and reception
         */
        uint32_t fcs_init2f : 1;
        //
        uint32_t phr_mode   : 2;        // This configuration allows selection of PHR type to be one of two options.
        uint32_t dis_stxp   : 1;        // Disable Smart TX Power control
        uint32_t rsvd1      : 3;        // Reserved
        uint32_t rxm110k    : 1;        // Receiver Mode 110 kbps data rate
        uint32_t rsvd2      : 5;        // Reserved
        uint32_t rxwtoe     : 1;        // Receive Wait Timeout Enable
        uint32_t rxautr     : 1;        // Receiver Auto-Re-enable
        uint32_t autoack    : 1;        // Automatic Acknowledgement Enable
        uint32_t aackpend   : 1;        // Automatic Acknowledgement Pending bit control
    };
    uint32_t value;
};

// Register file: 0x05 – Reserved

// Register file: 0x06 – System Time Counter
union dw1000_reg_sys_time
{
    uint8_t sys_time[5];                // System Time Counter (40-bit)
    struct
    {
        uint32_t sys_time_l;
        uint8_t sys_time_h;
    };
    uint8_t value[5];
};

// Register file: 0x07 – Reserved

// REG:08:00 – TX_FCTRL – Transmit Frame Control (Octets 0 to 3, 32-bits)
union dw1000_reg_tx_fctrl_08_00
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
    uint32_t value;
};

// REG:08:04 – TX_FCTRL – Transmit Frame Control (Octet 4, 8-bits)
union dw1000_reg_tx_fctrl_08_04
{
    struct
    {
        uint8_t ifsdelay : 8;           // Inter-Frame Spacing.
    };
    uint8_t value;
};

// Register file: 0x08 – Transmit Frame Control
union dw1000_reg_tx_fctrl
{
    struct
    {
        union dw1000_reg_tx_fctrl_08_00 ofs_00;
        union dw1000_reg_tx_fctrl_08_04 ofs_04;
    };
    uint8_t value[5];
};

// Register file: 0x0A – Delayed Send or Receive Time
union dw1000_reg_dx_time
{
    struct
    {
        uint32_t dx_time_l;             // Delayed Send or Receive Time (40-bit).
        uint8_t dx_time_h;              // Delayed Send or Receive Time (40-bit).
    };
    uint8_t value[5];
};

// Register file: 0x0B – Reserved

// Register file: 0x0C – Receive Frame Wait Timeout Period

// Register file: 0x0D – System Control Register
union dw1000_reg_sys_ctrl
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

// Register file: 0x0E – System Event Mask Register
union dw1000_reg_sys_mask
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
        uint32_t mrxphe    : 1;         // Bit[12] Mask receiver PHY header error event.
        uint32_t mrxdfr    : 1;         // Bit[13] Mask receiver data frame ready event.
        uint32_t mrxfcg    : 1;         // Bit[14] Mask receiver FCS good event.
        uint32_t mrxfce    : 1;         // Bit[15] Mask receiver FCS error event.
        //
        uint32_t mrxrfsl   : 1;         // Bit[16] Mask receiver Reed Solomon Frame Sync Loss event.
        uint32_t mrxrfto   : 1;         // Bit[17] Mask Receive Frame Wait Timeout event.
        uint32_t mldeerr   : 1;         // Bit[18] Mask leading edge detection processing error event.
        uint32_t rsvd2     : 1;         // Bit[19] Reserved.
        uint32_t mrxovrr   : 1;         // Bit[20] Mask Receiver Overrun event.
        uint32_t mrxpto    : 1;         // Bit[21] Mask Preamble detection timeout event.
        uint32_t mgpioirq  : 1;         // Bit[22] Mask GPIO interrupt event.
        uint32_t mslp2init : 1;         // Bit[23] Mask SLEEP to INIT event.
        //
        uint32_t mrfpllll  : 1;         // Bit[24] Mask RF PLL Losing Lock warning event.
        uint32_t mcpllll   : 1;         // Bit[25] Mask Clock PLL Losing Lock warning event.
        uint32_t mrxsfdto  : 1;         // Bit[26] Mask Receive SFD timeout event.
        uint32_t mhpdwarn  : 1;         // Bit[27] Mask Half Period Delay Warning event.
        uint32_t mtxberr   : 1;         // Bit[28] Mask Transmit Buffer Error event.
        uint32_t maffrej   : 1;         // Bit[29] Mask Automatic Frame Filtering rejection event.
        uint32_t rsvd3     : 2;         // Bit[31:30] Reserved.
    };
    uint32_t value;
};

// REG:0F:00 – SYS_STATUS – System Status Register (octets 0 to 3)
union dw1000_reg_sys_status_0f_00
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
        uint32_t rxphe     : 1;         // Bit[12] Receiver PHY Header Error.
        uint32_t rxdfr     : 1;         // Bit[13] Receiver Data Frame Ready.
        uint32_t rxfcg     : 1;         // Bit[14] Receiver FCS Good.
        uint32_t rxfce     : 1;         // Bit[15] Receiver FCS Error.
        //
        uint32_t rxrfsl    : 1;         // Bit[16] Receiver Reed Solomon Frame Sync Loss.
        uint32_t rxrfto    : 1;         // Bit[17] Receive Frame Wait Timeout.
        uint32_t ldeerr    : 1;         // Bit[18] Leading edge detection processing error.
        uint32_t rsvd      : 1;         // Bit[19] Reserved.
        uint32_t rxovrr    : 1;         // Bit[20] Receiver Overrun.
        uint32_t rxpto     : 1;         // Bit[21] Preamble detection timeout.
        uint32_t gpioirq   : 1;         // Bit[22] GPIO interrupt.
        uint32_t slp2init  : 1;         // Bit[23] SLEEP to INIT.
        //
        uint32_t rfpll_ll  : 1;         // Bit[24] RF PLL Losing Lock.
        uint32_t clkpll_ll : 1;         // Bit[25] Clock PLL Losing Lock.
        uint32_t rxsfdto   : 1;         // Bit[26] Receive SFD timeout.
        uint32_t hpdwarn   : 1;         // Bit[27] Half Period Delay Warning.
        uint32_t txberr    : 1;         // Bit[28] Transmit Buffer Error.
        uint32_t affrej    : 1;         // Bit[29] Automatic Frame Filtering rejection.
        uint32_t hsrbp     : 1;         // Bit[30] Host Side Receive Buffer Pointer.
        uint32_t icrbp     : 1;         // Bit[31] IC side Receive Buffer Pointer.
    };
    uint32_t value;
};

// REG:0F:04 – SYS_STATUS – System Status Register (octet 4)
union dw1000_reg_sys_status_0f_04
{
    struct
    {
        uint8_t rxrscs : 1;             // Bit[0] Receiver Reed-Solomon Correction Status.
        uint8_t rxprej : 1;             // Bit[1] Receiver Preamble Rejection.
        uint8_t txpute : 1;             // Bit[2] Transmit power up time error.
        uint8_t rsvd   : 5;             // Bit[7:3] Reserved.
    };
    uint8_t value;
};

// Register file: 0x0F – System Event Status Register
union dw1000_reg_sys_status
{
    struct
    {
        union dw1000_reg_sys_status_0f_00 ofs_00;
        union dw1000_reg_sys_status_0f_04 ofs_04;
    };
    uint8_t value[5];
};

// Register file: 0x10 – RX Frame Information (in double buffer set)
union dw1000_reg_rx_finfo
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

// // Transmit Bit Rate Selection (Data Rate)
// enum dw1000_rxbr_sel
// {
//     TXBR_110KBPS  = 0,
//     TXBR_850KBPS  = 1,
//     TXBR_6800KBPS = 2,
//     TXBR_RSVD     = 3,
// };

// // Transmit Pulse Repetition Frequency Selection (PRF)
// enum dw1000_rxprf_sel
// {
//     TXPRF_4MHZ  = 0,
//     TXPRF_16MHZ = 1,
//     TXPRF_64MHZ = 2,
//     TXPRF_RSVD  = 3,
// };

// // Transmit Preamble Symbol Repetitions Selection (Preamble Length)
// enum dw1000_rxpsr_sel
// {
//     TXPSR_64   = 0x1,  // The standard preamble length for the 802.15.4 UWB PHY
//     TXPSR_128  = 0x5,
//     TXPSR_256  = 0x9,
//     TXPSR_512  = 0xd,
//     TXPSR_1024 = 0x2,  // The standard preamble length for the 802.15.4 UWB PHY
//     TXPSR_1536 = 0x6,
//     TXPSR_2048 = 0xa,
//     TXPSR_4096 = 0x3,  // The standard preamble length for the 802.15.4 UWB PHY
// };

// Register file: 0x11 – Receive Data (in double buffer set)

union dw1000_reg_rx_fqual_12_00
{
    struct
    {
        uint32_t std_noise : 16;        // Bit[15:0] Standard Deviation of Noise.
        uint32_t fp_ampl2  : 16;        // Bit[31:16] First Path Amplitude point 2.
    };
    uint32_t value;
};

union dw1000_reg_rx_fqual_12_04
{
    struct
    {
        uint32_t fp_ampl3 : 16;         // Bit[15:0] First Path Amplitude point 3.
        uint32_t cir_pwr  : 16;         // Bit[31:16] Channel Impulse Response Power.
    };
    uint32_t value;
};

// Register file: 0x12 – Rx Frame Quality information (in double buffer set)
union dw1000_reg_rx_fqual
{
    struct
    {
        union dw1000_reg_rx_fqual_12_00 ofs_00;
        union dw1000_reg_rx_fqual_12_04 ofs_04;
    };
    uint64_t value;
};

// Register file: 0x13 – Receiver Time Tracking Interval (in double buffer set)

// Register file: 0x14 – Receiver Time Tracking Offset (in double buffer set)

// Register file: 0x15 – Receive Message Time of Arrival (in double buffer set)

// REG:15:00 – RX_TIME – Receive Time Stamp (Octets 0 to 3, 32-bits)
union dw1000_reg_rx_time_15_00
{
    struct
    {
        uint32_t rx_stamp_l;            // The fully adjusted time of reception (low 32 bits of 40-bit value).
    };
    uint32_t value;
};

// REG:15:04 – RX_TIME – Receive Time Stamp (Octets 4 to 7, 32-bits)
union dw1000_reg_rx_time_15_04
{
    struct
    {
        uint32_t rx_stamp_h : 8;        // The fully adjusted time of reception (high 8 bits of 40-bit value).
        uint32_t fp_index   : 16;       // First path index.
        uint32_t fp_ampl1_l : 8;        // First Path Amplitude point 1 (low 8 bits of 16-bit value).
    };
    uint32_t value;
};

// REG:15:08 – RX_TIME – Receive Time Stamp (Octets 8 to 11, 32-bits)
union dw1000_reg_rx_time_15_08
{
    struct
    {
        uint32_t fp_ampl1_h : 8;        // First Path Amplitude point 1 (high 8 bits of 16-bit value).
        uint32_t rx_rawst_l : 24;       // The Raw Timestamp for the frame (low 24 bits of 40-bit value).
    };
    uint32_t value;
};

// REG:15:0C – RX_TIME – Receive Time Stamp (Octets 12 to 13, 16-bits)
union dw1000_reg_rx_time_15_0c
{
    struct
    {
        uint16_t rx_rawst_h;            // The Raw Timestamp for the frame (high 16 bits of 40-bit value).
    };
    uint16_t value;
};

union dw1000_reg_rx_time
{
    struct
    {
        union dw1000_reg_rx_time_15_00 ofs_00;
        union dw1000_reg_rx_time_15_04 ofs_04;
        union dw1000_reg_rx_time_15_08 ofs_08;
        union dw1000_reg_rx_time_15_0c ofs_0c;
    };
    uint8_t value[14];
};

// Register file: 0x16 – Reserved

// Register file: 0x17 – Transmit Time Stamp
union dw1000_reg_tx_time_stamp
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

// Register file: 0x18 – Transmitter Antenna Delay
union dw1000_reg_tx_antd
{
    struct
    {
        uint16_t tx_antdl;              // 16-bit Delay from Transmit to Antenna.
    };
    uint16_t value;
};

// Register file: 0x19 – DW1000 State Information

// Register file: 0x1A – Acknowledgement time and response time

// Register file: 0x1B – Reserved

// Register file: 0x1C – Reserved

// Register file: 0x1D – SNIFF Mode

// Register file: 0x1E – Transmit Power Control

// Register file: 0x1F – Channel Control
union dw1000_reg_chan_ctrl
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

// Register file: 0x20 – Reserved

// Register file: 0x21 – User defined SFD sequence

// Register file: 0x22 – Reserved

// Register file: 0x23 –AGC configuration and control

// Register file: 0x24 – External Synchronisation Control

// Register file: 0x25 – Accumulator CIR memory

// Register file: 0x26 – GPIO control and status

// Sub-Register 0x27:00 – DRX_RES1

// Sub-Register 0x27:02 – DRX_TUNE0b

// Sub-Register 0x27:04 – DRX_TUNE1a

// Sub-Register 0x27:06 – DRX_TUNE1b

// Sub-Register 0x27:08 – DRX_TUNE2
union dw1000_sub_reg_drx_tune2
{
    struct
    {
        uint32_t drx_tun2;              // Digital Tuning Register 2. RW
    };
    uint32_t value;
};

// Sub-Register 0x27:0C – DRX_RES2

// Sub-Register 0x27:20 – DRX_SFDTOC

// Sub-Register 0x27:22 – DRX_RES3

// Sub-Register 0x27:24 – DRX_PRETOC
union dw1000_sub_reg_drx_pretoc
{
    struct
    {
        uint16_t drx_pretoc;            // Preamble detection timeout count (in units of PAC size symbols)
    };
    uint16_t value;
};

// Sub-Register 0x27:26 – DRX_TUNE4H

// Sub-Register 0x27:28 – DRX_CAR_INT

// Sub-Register 0x27:2C – RXPACC_NOSAT

// Register file: 0x27 – Digital receiver configuration

// Sub-Register 0x28:00 – RF_CONF

// Sub-Register 0x28:04 – RF_RES1

// Sub-Register 0x28:0B – RF_RXCTRLH

// Sub-Register 0x28:0C – RF_TXCTRL

// Sub-Register 0x28:10 – RF_RES2

// Sub-Register 0x28:2C – RF_STATUS

// Sub-Register 0x28:30 – LDOTUNE

// Register file: 0x28 – Analog RF configuration block

// Register file: 0x29 – Reserved

// Sub-Register 0x2A:00 – TC_SARC

// Sub-Register 0x2A:03 – TC_SARL

// Sub-Register 0x2A:06 – TC_SARW

// Sub-Register 0x2A:08 – TC_PG_CTRL

// Sub-Register 0x2A:09 – TC_PG_STATUS

// Sub-Register 0x2A:0B – TC_PGDELAY

// Sub-Register 0x2A:0C – TC_PGTEST

// Register file: 0x2A – Transmitter Calibration block

// Sub-Register 0x2B:00 – FS_RES1

// Sub-Register 0x2B:07 – FS_PLLCFG

// Sub-Register 0x2B:0B – FS_PLLTUNE

// Sub-Register 0x2B:0C – FS_RES2

// Sub-Register 0x2B:0E – FS_XTALT

// Sub-Register 0x2B:0F – FS_RES3

// Register file: 0x2B – Frequency synthesiser control block

// Sub-Register 0x2C:00 – AON_WCFG

// Sub-Register 0x2C:02 – AON_CTRL

// Sub-Register 0x2C:03 – AON_RDAT

// Sub-Register 0x2C:04 – AON_ADDR

// Sub-Register 0x2C:05 – AON_RES1

// Sub-Register 0x2C:06 – AON_CFG0
union dw1000_sub_reg_aon_cfg0
{
    struct
    {
        uint32_t sleep_en : 1;          // Bit[0] Sleep Enable.
        uint32_t wake_pin : 1;          // Bit[1] Wake using WAKEUP pin.
        uint32_t wake_spi : 1;          // Bit[2] Wake using SPI access.
        uint32_t wake_cnt : 1;          // Bit[3] Wake when sleep counter elapses.
        uint32_t lpdiv_en : 1;          // Bit[4] Low power divider enable configuration.
        uint32_t lpclkdiva : 11;        // Bit[15:5] divider count for dividing the raw DW1000 XTAL oscillator frequency.
        uint32_t sleep_tim : 16;        // Bit[31:16] Sleep time.
    };
    uint32_t value;
};

// Sub-Register 0x2C:0A – AON_CFG1

// Register file: 0x2C – Always-on system control interface

// Sub-Register 0x2D:00 – OTP_WDAT

// Sub-Register 0x2D:04 – OTP_ADDR

// Sub-Register 0x2D:06 – OTP_CTRL

// Sub-Register 0x2D:08 – OTP_STAT

// Sub-Register 0x2D:0A – OTP_RDAT

// Sub-Register 0x2D:0E – OTP_SRDAT

// Sub-Register 0x2D:12 – OTP_SF

// Register file: 0x2D – OTP Memory Interface

// Sub-Register 0x2E:0000 – LDE_THRESH

// Sub-Register 0x2E:0806 – LDE_CFG1

// Sub-Register 0x2E:1000 – LDE_PPINDX

// Sub-Register 0x2E:1002 – LDE_PPAMPL

// Sub-Register 0x2E:1804 – LDE_RXANTD

// Sub-Register 0x2E:1806 – LDE_CFG2

// Sub-Register 0x2E:2804 – LDE_REPC

// Register file: 0x2E – Leading Edge Detection Interface

// Sub-Register 0x2F:00 – Event Counter Control
union dw1000_sub_reg_evc_ctrl
{
    struct
    {
        uint32_t evc_en  : 1;           // Event Counters Enable. SRW
        uint32_t evc_clr : 1;           // Event Counters Clear. SRW
        uint32_t rsvd    : 30;          // Reserved
    };
    uint32_t value;
};

// Sub-Register 0x2F:04 – PHR Error Counter
union dw1000_sub_reg_evc_phe
{
    struct
    {
        uint16_t evc_phe : 12;          // PHR Error Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:06 – RSD Error Counter
union dw1000_sub_reg_evc_rse
{
    struct
    {
        uint16_t evc_rse : 12;          // Reed Solomon decoder (Frame Sync Loss) Error Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:08 – FCS Good Counter
union dw1000_sub_reg_evc_fcg
{
    struct
    {
        uint16_t evc_fcg : 12;          // Frame Check Sequence Good Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:0A – FCS Error Counter
union dw1000_sub_reg_evc_fce
{
    struct
    {
        uint16_t evc_fce : 12;          // Frame Check Sequence Error Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:0C – Frame Filter Rejection Counter
union dw1000_sub_reg_evc_ffr
{
    struct
    {
        uint16_t evc_ffr : 12;          // Frame Filter Rejection Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:0E – RX Overrun Error Counter
union dw1000_sub_reg_evc_ovr
{
    struct
    {
        uint16_t evc_ovr : 12;          // RX Overrun Error Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:10 – SFD Timeout Error Counter
union dw1000_sub_reg_evc_sto
{
    struct
    {
        uint16_t evc_sto : 12;          // SFD (start of frame delimiter) timeout errors Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:12 – Preamble Detection Timeout Event Counter
union dw1000_sub_reg_evc_pto
{
    struct
    {
        uint16_t evc_pto : 12;          // Preamble Detection Timeout Event Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:14 – RX Frame Wait Timeout Event Counter
union dw1000_sub_reg_evc_fwto
{
    struct
    {
        uint16_t evc_fwto : 12;         // RX Frame Wait Timeout Event Counter. RO
        uint16_t rsvd     : 4;          // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:16 – TX Frame Sent Counter
union dw1000_sub_reg_evc_txfs
{
    struct
    {
        uint16_t evc_txfs : 12;         // TX Frame Sent Event Counter. RO
        uint16_t rsvd     : 4;          // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:18 – Half Period Warning Counter
union dw1000_sub_reg_evc_hpw
{
    struct
    {
        uint16_t rvc_hpw : 12;          // Half Period Warning Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
};

// Sub-Register 0x2F:1A – Transmitter Power-Up Warning Counter
union dw1000_sub_reg_evc_tpw
{
    struct
    {
        uint16_t evc_tpw : 12;          // Transmitter Power-Up Warning Counter. RO
        uint16_t rsvd    : 4;           // Reserved
    };
    uint16_t value;
};

// Sub-Register 0x2F:1C – Digital Diagnostics Reserved Area 1
union dw1000_sub_reg_evc_res1
{
    struct
    {
        uint64_t evc_res1;              // Digital Diagnostics Reserved Area 1. RW
    };
    uint64_t value;
};

// Sub-Register 0x2F:24 – Test Mode Control Register
union dw1000_sub_reg_diag_tmc
{
    struct
    {
        uint16_t ravd1   : 4;           // Reserved 1
        uint16_t tx_pstm : 1;           // Transmit Power Spectrum Test Mode.
        uint16_t rsvd2   : 11;          // Reserved 2
    };
    uint16_t value : 16;
};

// Register file: 0x2F – Digital Diagnostics Interface
union dw1000_reg_dig_diag
{
    struct
    {
        union dw1000_sub_reg_evc_ctrl evc_ctrl; // Sub-Register 0x2F:00 – Event Counter Control
        union dw1000_sub_reg_evc_phe evc_phe;   // Sub-Register 0x2F:04 – PHR Error Counter
        union dw1000_sub_reg_evc_rse evc_rse;   // Sub-Register 0x2F:06 – RSD Error Counter
        union dw1000_sub_reg_evc_fcg evc_fcg;   // Sub-Register 0x2F:08 – FCS Good Counter
        union dw1000_sub_reg_evc_fce evc_fce;   // Sub-Register 0x2F:0A – FCS Error Counter
        union dw1000_sub_reg_evc_ffr evc_ffr;   // Sub-Register 0x2F:0C – Frame Filter Rejection Counter
        union dw1000_sub_reg_evc_ovr evc_ovr;   // Sub-Register 0x2F:0E – RX Overrun Error Counter
        union dw1000_sub_reg_evc_sto evc_sto;   // Sub-Register 0x2F:10 – SFD Timeout Error Counter
        union dw1000_sub_reg_evc_pto evc_pto;   // Sub-Register 0x2F:12 – Preamble Detection Timeout Event Counter
        union dw1000_sub_reg_evc_fwto evc_fwto; // Sub-Register 0x2F:14 – RX Frame Wait Timeout Event Counter
        union dw1000_sub_reg_evc_txfs evc_txfs; // Sub-Register 0x2F:16 – TX Frame Sent Counter
        union dw1000_sub_reg_evc_hpw evc_hpw;   // Sub-Register 0x2F:18 – Half Period Warning Counter
        union dw1000_sub_reg_evc_tpw evc_tpw;   // Sub-Register 0x2F:1A – Transmitter Power-Up Warning Counter
        union dw1000_sub_reg_evc_res1 evc_res1; // Sub-Register 0x2F:1C – EVC_RES1
        union dw1000_sub_reg_diag_tmc diag_tmc; // Sub-Register 0x2F:24 – Digital Diagnostics Test Mode Control
        uint8_t rsvd[3];
    };
    uint8_t value[41];
};

// Register files: 0x30 to 0x35 – Reserved

// Sub-Register 0x36:00 – PMSC_CTRL0

// Sub-Register 0x36:04 – PMSC_CTRL1

// Sub-Register 0x36:08 – PMSC_RES1

// Sub-Register 0x36:0C – PMSC_SNOZT

// Sub-Register 0x36:10 – PMSC_RES2

// Sub-Register 0x36:26 – PMSC_TXFSEQ

// Sub-Register 0x36:28 – PMSC_LEDC

// Register file: 0x36 – Power Management and System Control
union dw1000_reg_pmsc
{

};

// Register files: 0x37 to 0x3F – Reserved

#pragma pop

// Single octet header of the non-indexed SPI transaction
union dw1000_tran_header1
{
    struct
    {
        uint8_t rid : 6;                // Register file ID – Range 0x00 to 0x3F (64 locations)
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
        uint16_t rid      : 6;          // Register file ID – Range 0x00 to 0x3F (64 locations)
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
        uint8_t rid         : 6;        // Register file ID – Range 0x00 to 0x3F (64 locations)
        uint8_t si          : 1;        // Bit = 1, says sub-index is not present
        uint8_t op          : 1;        // Operation: 0 = Read, 1 = Write
        uint16_t sub_addr_l : 7;        // Low order 7 bits of 15-bit Register file sub-address, range 0x0000 to 0x7FFF (32768 byte locations)
        uint16_t ext        : 1;        // Extended Address: 1 = yes
        uint16_t sub_addr_h : 8;        // High order 8 bits of 15-bit Register file sub-address, range 0x0000 to 0x7FFF (32768 byte locations)
    };
    uint8_t value[3];
};

struct dw1000_reg
{
    const char *mnemonic;
    const char *desc;
    uint16_t length;
    uint8_t reg_file_id;
    uint8_t reg_file_type;
};

struct dw1000_trx_para
{
    uint8_t tx_chan;                    // Transmit channel
    uint8_t rx_chan;                    // Receive channel
    uint8_t tx_pcode;                   // Preamble code used in the transmitter
    uint8_t rx_pcode;                   // Preamble code used in the receiver
    uint16_t tflen;                     // Transmit Frame Length (Data Length)
    enum dw1000_br_sel txbr_sel;        // Data Rate
    enum dw1000_prf_sel txprf_sel;      // PRF
    enum dw1000_psr_sel txpsr_sel;      // Preamble Length
    enum dw1000_br_sel rxbr_sel;        // Data Rate
    enum dw1000_prf_sel rxprf_sel;      // PRF
    enum dw1000_psr_sel rxpsr_sel;      // Preamble Length
    uint16_t buf_ofs;                   // Transmit buffer index offset
    uint8_t ifs_delay;                  // Inter-Frame Spacing (Delay)
    uint64_t txdlys;                    // Delayed Send Time (Unit: 15.65 picoseconds)
    //
    uint32_t drx_tune2;                 // Digital Tuning Register 2 (for PAC size and RXPRF)
    uint32_t drx_pretoc;                // Preamble detection timeout count (in units of PAC size symbols)
    uint32_t drx_sfdtoc;                // SFD detection timeout count (Default: 4096 + 64 + 1 symbols)
    uint64_t rxdlye;                    // Delayed Receive Time (Unit: 15.65 picoseconds)
};

struct dw1000_context
{
    struct spi_config spi_cfg;
    struct gpio_irq_config irq_cfg;
    struct dw1000_trx_para trx_cfg;
};

void dw1000_ctx_init(struct dw1000_context *dw1000_ctx);
void dw1000_spi_master_test(const struct dw1000_context *dw1000_ctx);

#endif  // ~ DW1000_H
