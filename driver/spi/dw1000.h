#ifndef DW1000_H
#define DW1000_H

#include <stdbool.h>
#include <stdint.h>

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

enum dw1000_dig_diag_sub_reg_ofs
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

enum dw1000_drx_conf_sub_reg_ofs
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
    UWB_BEACON_FRAME = 0,
    UWB_DATA_FRAME = 1,
    UWB_ACK_FRAME = 2,
    UWB_MAC_CMD_FRAME = 3,
    UWB_RSVD_FRAME1 = 4,
    UWB_RSVD_FRAME2 = 5,
    UWB_RSVD_FRAME3 = 6,
    UWB_RSVD_FRAME4 = 7,
};

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

// Transmit Bit Rate Selection (Data Rate)
enum dw1000_txbr_sel
{
    TXBR_110KBPS  = 0,
    TXBR_850KBPS  = 1,
    TXBR_6800KBPS = 2,
    TXBR_RSVD     = 3,
};

// Transmit Pulse Repetition Frequency Selection (PRF)
enum dw1000_txprf_sel
{
    TXPRF_4MHZ  = 0,
    TXPRF_16MHZ = 1,
    TXPRF_64MHZ = 2,
    TXPRF_RSVD  = 3,
};

// Transmit Preamble Symbol Repetitions Selection (Preamble Length)
enum dw1000_txpsr_sel
{
    TXPSR_64   = 0x1,  // The standard preamble length for the 802.15.4 UWB PHY
    TXPSR_128  = 0x5,
    TXPSR_256  = 0x9,
    TXPSR_512  = 0xd,
    TXPSR_1024 = 0x2,  // The standard preamble length for the 802.15.4 UWB PHY
    TXPSR_1536 = 0x6,
    TXPSR_2048 = 0xa,
    TXPSR_4096 = 0x3,  // The standard preamble length for the 802.15.4 UWB PHY
};

// REG:08:04 – TX_FCTRL – Transmit Frame Control (Octet 4, 8-bits)
union dw1000_reg_tx_fctrl_08_04
{
    struct
    {
        uint8_t ifsdelay : 8;           // Inter-Frame Spacing
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
    uint8_t dx_time[5];                 // Delayed Send or Receive Time (40-bit)
    struct
    {
        uint32_t dx_time_l;
        uint8_t dx_time_h;
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
        uint32_t sfcst     : 1;         // Suppress auto-FCS Transmission (on this next frame)
        uint32_t txstrt    : 1;         // Transmit Start
        uint32_t txdlys    : 1;         // Transmitter Delayed Sending
        uint32_t cansfcs   : 1;         // Cancel Suppression of auto-FCS transmission (on the current frame)
        uint32_t rsvd1     : 2;         // Reserved
        uint32_t trxoff    : 1;         // Transceiver Off
        uint32_t wait4resp : 1;         // Wait for Response
        //
        uint32_t rxenab    : 1;         // Enable Receiver
        uint32_t rxdlye    : 1;         // Receiver Delayed Enable
        uint32_t rsvd2     : 14;        // Reserved
        //
        uint32_t hrbpt     : 1;         // Host Side Receive Buffer Pointer Toggle
        uint32_t rsvd3     : 7;         // Reserved
    };
    uint32_t value;
};

// REG:0F:00 – SYS_STATUS – System Status Register (octets 0 to 3)
union dw1000_reg_sys_status_0f_00
{
    struct
    {
        uint32_t irqs      : 1;         // Interrupt Request Status.
        uint32_t cplock    : 1;         // Clock PLL Lock
        uint32_t esyncr    : 1;         // External Sync Clock Reset
        uint32_t aat       : 1;         // Automatic Acknowledge Trigger
        uint32_t txfrb     : 1;         // Transmit Frame Begins
        uint32_t txprs     : 1;         // Transmit Preamble Sent
        uint32_t txphs     : 1;         // bit[6] Transmit PHY Header Sent
        uint32_t txfrs     : 1;         // Transmit Frame Sent
        //
        uint32_t rxprd     : 1;         // Receiver Preamble Detected status
        uint32_t rxsfdd    : 1;         // Receiver SFD Detected
        uint32_t ldedone   : 1;         // LDE processing done
        uint32_t rxphd     : 1;         // Receiver PHY Header Detect
        uint32_t rxphe     : 1;         // Receiver PHY Header Error
        uint32_t rxdfr     : 1;         // bit[13] Receiver Data Frame Ready
        uint32_t rxfcg     : 1;         // Receiver FCS Good
        uint32_t rxfce     : 1;         // Receiver FCS Error
        //
        uint32_t rxrfsl    : 1;         // Receiver Reed Solomon Frame Sync Loss
        uint32_t rxrfto    : 1;         // Receive Frame Wait Timeout
        uint32_t ldeerr    : 1;         // Leading edge detection processing error
        uint32_t rsvd      : 1;         // Reserved
        uint32_t rxovrr    : 1;         // Receiver Overrun
        uint32_t rxpto     : 1;         // Preamble detection timeout
        uint32_t gpioirq   : 1;         // GPIO interrupt
        uint32_t slp2init  : 1;         // SLEEP to INIT
        //
        uint32_t rfpll_ll  : 1;         // RF PLL Losing Lock
        uint32_t clkpll_ll : 1;         // Clock PLL Losing Lock
        uint32_t rxsfdto   : 1;         // Receive SFD timeout
        uint32_t hpdwarn   : 1;         // Half Period Delay Warning
        uint32_t txberr    : 1;         // Transmit Buffer Error
        uint32_t affrej    : 1;         // Automatic Frame Filtering rejection
        uint32_t hsrbp     : 1;         // Host Side Receive Buffer Pointer
        uint32_t icrbp     : 1;         // IC side Receive Buffer Pointer
    };
    uint32_t value;
};

// REG:0F:04 – SYS_STATUS – System Status Register (octet 4)
union dw1000_reg_sys_status_0f_04
{
    struct
    {
        uint8_t rxrscs : 1;             // Receiver Reed-Solomon Correction Status
        uint8_t rxprej : 1;             // Receiver Preamble Rejection
        uint8_t txpute : 1;             // Transmit power up time error
        uint8_t rsvd   : 5;             // Reserved
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

// Register file: 0x11 – Receive Data (in double buffer set)

// Register file: 0x12 – Rx Frame Quality information (in double buffer set)

// Register file: 0x13 – Receiver Time Tracking Interval (in double buffer set)

// Register file: 0x14 – Receiver Time Tracking Offset (in double buffer set)

// Register file: 0x15 – Receive Message Time of Arrival (in double buffer set)
union dw1000_reg_rx_time
{
    struct
    {
        uint8_t rx_time[14];
    };
    uint8_t value[14];
};

// Register file: 0x16 – Reserved

// Register file: 0x17 – Transmit Time Stamp
union dw1000_reg_tx_time_stamp
{
    struct
    {
        uint32_t tx_stamp_l;            // This 40-bit (5-octet) field reports the fully adjusted time of transmission.
        uint32_t tx_stamp_h : 8;        // This 40-bit (5-octet) field reports the fully adjusted time of transmission.
        uint32_t tx_rawst_l : 24;       // This 40-bit (5-octet) field reports the Raw Timestamp for the frame.
        uint16_t tx_rawst_h;            // This 40-bit (5-octet) field reports the Raw Timestamp for the frame.
    };
    uint8_t value[10];
};

// Register file: 0x18 – Transmitter Antenna Delay
union dw1000_reg_tx_antd
{
    struct
    {
        uint16_t tx_antdl;              // 16-bit Delay from Transmit to Antenna
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

// Register file: 0x20 – Reserved

// Register file: 0x21 – User defined SFD sequence

// Register file: 0x22 – Reserved

// Register file: 0x23 –AGC configuration and control

// Register file: 0x24 – External Synchronisation Control

// Register file: 0x25 – Accumulator CIR memory

// Register file: 0x26 – GPIO control and status

// Sub-Register 0x27:08 – DRX_TUNE2
union dw1000_sub_reg_drx_tune2
{
    struct
    {
        uint32_t drx_tun2;              // Digital Tuning Register 2. RW
    };
    uint32_t value;
};

enum drx_tune2_value
{
    PAC_8_PRF_16MHZ  = 0x311A002D,
    PAC_8_PRF_64MHZ  = 0x313B006B,
    PAC_16_PRF_16MHZ = 0x331A0052,
    PAC_16_PRF_64MHZ = 0x333B00BE,
    PAC_32_PRF_16MHZ = 0x351A009A,
    PAC_32_PRF_64MHZ = 0x353B015E,
    PAC_64_PRF_16MHZ = 0x371A011D,
    PAC_64_PRF_64MHZ = 0x373B0296,
};

// Register file: 0x27 – Digital receiver configuration

// Register file: 0x28 – Analog RF configuration block

// Register file: 0x29 – Reserved

// Register file: 0x2A – Transmitter Calibration block

// Register file: 0x2B – Frequency synthesiser control block

// Register file: 0x2C – Always-on system control interface

// Register file: 0x2D – OTP Memory Interface

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
        uint16_t sub_addr_h : 16;       // High order 8 bits of 15-bit Register file sub-address, range 0x0000 to 0x7FFF (32768 byte locations)
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

void dw1000_spi_master_test();

#endif  // ~ DW1000_H
