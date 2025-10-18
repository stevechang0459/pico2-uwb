/**
 * Copyright (c) 2025 Steve Chang
 *
 * SPDX-License-Identifier: MIT
 */

#include "dw1000.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "spi.h"
#include "led.h"
#include "print.h"
#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <float.h>

static bool led_out = 0;
static struct dw1000_context m_dw1000_ctx = {0};

#define DW1000_SUB_REG_DESC(m, t, s) \
    {.reg_file_id = DW1000_##m, .length = sizeof(union DW1000_SUB_REG_##m), .reg_file_type = DW1000_##t, .mnemonic = #m, .desc = s}

static struct dw1000_reg dw1000_regs[] =
{
    {.reg_file_id = DW1000_DEV_ID,      .length = 4,    .reg_file_type = DW1000_RO,  .mnemonic = "DEV_ID",     .desc = "Device Identifier"},
    {.reg_file_id = DW1000_EUI,         .length = 8,    .reg_file_type = DW1000_RW,  .mnemonic = "EUI",        .desc = "Extended Unique Identifier"},
    {.reg_file_id = 0x02, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_PANADR,      .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "PANADR",     .desc = "PAN Identifier and Short Address"},
    {.reg_file_id = DW1000_SYS_CFG,     .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "SYS_CFG",    .desc = "System Configuration bitmap"},
    {.reg_file_id = 0x05, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_SYS_TIME,    .length = 5,    .reg_file_type = DW1000_RO,  .mnemonic = "SYS_TIME",   .desc = "System Time Counter"},
    {.reg_file_id = 0x07, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_TX_FCTRL,    .length = 5,    .reg_file_type = DW1000_RW,  .mnemonic = "TX_FCTRL",   .desc = "Transmit Frame Control"},
    {.reg_file_id = DW1000_TX_BUFFER,   .length = 1024, .reg_file_type = DW1000_WO,  .mnemonic = "TX_BUFFER",  .desc = "Transmit Data Buffer"},
    {.reg_file_id = DW1000_DX_TIME,     .length = 5,    .reg_file_type = DW1000_RW,  .mnemonic = "DX_TIME",    .desc = "Delayed Send or Receive Time"},
    {.reg_file_id = 0x0b, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_RX_FWTO,     .length = 2,    .reg_file_type = DW1000_RW,  .mnemonic = "RX_FWTO",    .desc = "Receive Frame Wait Timeout Period"},
    {.reg_file_id = DW1000_SYS_CTRL,    .length = 4,    .reg_file_type = DW1000_SRW, .mnemonic = "SYS_CTRL",   .desc = "System Control Register"},
    {.reg_file_id = DW1000_SYS_MASK,    .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "SYS_MASK",   .desc = "System Event Mask Register"},
    {.reg_file_id = DW1000_SYS_STATUS,  .length = 5,    .reg_file_type = DW1000_SRW, .mnemonic = "SYS_STATUS", .desc = "System Event Status Register"},
    {.reg_file_id = DW1000_RX_FINFO,    .length = 4,    .reg_file_type = DW1000_ROD, .mnemonic = "RX_FINFO",   .desc = "RX Frame Information"},
    {.reg_file_id = DW1000_RX_BUFFER,   .length = 1024, .reg_file_type = DW1000_ROD, .mnemonic = "RX_BUFFER",  .desc = "Receive Data"},
    {.reg_file_id = DW1000_RX_FQUAL,    .length = 8,    .reg_file_type = DW1000_ROD, .mnemonic = "RX_FQUAL",   .desc = "Rx Frame Quality information"},
    {.reg_file_id = DW1000_RX_TTCKI,    .length = 4,    .reg_file_type = DW1000_ROD, .mnemonic = "RX_TTCKI",   .desc = "Receiver Time Tracking Interval"},
    {.reg_file_id = DW1000_RX_TTCKO,    .length = 5,    .reg_file_type = DW1000_ROD, .mnemonic = "RX_TTCKO",   .desc = "Receiver Time Tracking Offset"},
    {.reg_file_id = DW1000_RX_TIME,     .length = 14,   .reg_file_type = DW1000_ROD, .mnemonic = "RX_TIME",    .desc = "Receive Message Time of Arrival"},
    {.reg_file_id = 0x16, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_TX_TIME,     .length = 10,   .reg_file_type = DW1000_RO,  .mnemonic = "TX_TIME",    .desc = "Transmit Message Time of Sending"},
    {.reg_file_id = DW1000_TX_ANTD,     .length = 2,    .reg_file_type = DW1000_RW,  .mnemonic = "TX_ANTD",    .desc = "16-bit Delay from Transmit to Antenna"},
    {.reg_file_id = DW1000_SYS_STATE,   .length = 5,    .reg_file_type = DW1000_RO,  .mnemonic = "SYS_STATE",  .desc = "System State information"},
    {.reg_file_id = DW1000_ACK_RESP_T,  .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "ACK_RESP_T", .desc = "Acknowledgement Time and Response Time"},
    {.reg_file_id = 0x1b, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x1c, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_RX_SNIFF,    .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "RX_SNIFF",   .desc = "SNIFF Mode"},
    {.reg_file_id = DW1000_TX_POWER,    .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "TX_POWER",   .desc = "TX Power Control"},
    {.reg_file_id = DW1000_CHAN_CTRL,   .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "CHAN_CTRL",  .desc = "Channel Control"},
    {.reg_file_id = 0x20, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_USR_SFD,     .length = 41,   .reg_file_type = DW1000_RW,  .mnemonic = "USR_SFD",    .desc = "User-specified short/long TX/RX SFD sequences"},
    {.reg_file_id = 0x22, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_AGC_CTRL,    .length = 33,   .reg_file_type = DW1000_RW,  .mnemonic = "AGC_CTRL",   .desc = "Automatic Gain Control configuration"},
    {.reg_file_id = DW1000_EXT_SYNC,    .length = 12,   .reg_file_type = DW1000_RW,  .mnemonic = "EXT_SYNC",   .desc = "External synchronisation control"},
    {.reg_file_id = DW1000_ACC_MEM,     .length = 4064, .reg_file_type = DW1000_RO,  .mnemonic = "ACC_MEM",    .desc = "Read access to accumulator data"},
    {.reg_file_id = DW1000_GPIO_CTRL,   .length = 44,   .reg_file_type = DW1000_RW,  .mnemonic = "GPIO_CTRL",  .desc = "Peripheral register bus 1 access"},
    {.reg_file_id = DW1000_DRX_CONF,    .length = 46,   .reg_file_type = DW1000_RW,  .mnemonic = "DRX_CONF",   .desc = "Digital Receiver configuration"},
    {.reg_file_id = DW1000_RF_CONF,     .length = 53/*58*/,   .reg_file_type = DW1000_RW,  .mnemonic = "RF_CONF",    .desc = "Analog RF Configuration"},
    {.reg_file_id = 0x29, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_TX_CAL,      .length = 52,   .reg_file_type = DW1000_RW,  .mnemonic = "TX_CAL",     .desc = "Transmitter calibration block"},
    {.reg_file_id = DW1000_FS_CTRL,     .length = 21,   .reg_file_type = DW1000_RW,  .mnemonic = "FS_CTRL",    .desc = "Frequency synthesiser control block"},
    {.reg_file_id = DW1000_AON,         .length = 12,   .reg_file_type = DW1000_RW,  .mnemonic = "AON",        .desc = "Always-On register set"},
    {.reg_file_id = DW1000_OTP_IF,      .length = 19/*18*/,   .reg_file_type = DW1000_RW,  .mnemonic = "OTP_IF",     .desc = "One Time Programmable Memory Interface"},
    {.reg_file_id = DW1000_LDE_CTRL,    .length = 0,    .reg_file_type = DW1000_RW,  .mnemonic = "LDE_CTRL",   .desc = "Leading edge detection control block"},
    {.reg_file_id = DW1000_DIG_DIAG,    .length = 41,   .reg_file_type = DW1000_RW,  .mnemonic = "DIG_DIAG",   .desc = "Digital Diagnostics Interface"},
    {.reg_file_id = 0x30, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x31, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x32, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x33, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x34, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = 0x35, .length = 0, .mnemonic = "Reserved", .desc = "Reserved"},
    {.reg_file_id = DW1000_PMSC,        .length = 48,   .reg_file_type = DW1000_RW,  .mnemonic = "PMSC",       .desc = "Power Management System Control Block"},
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

// Register file: 0x23 –AGC configuration and control overview
static struct dw1000_reg dw1000_agc_ctrl_sub_regs[] =
{
    DW1000_SUB_REG_DESC(AGC_RES1,  RO, "Reserved area 1"),
    DW1000_SUB_REG_DESC(AGC_CTRL1, RW, "AGC Control 1"),
    DW1000_SUB_REG_DESC(AGC_TUNE1, RW, "AGC Tuning register 1"),
    DW1000_SUB_REG_DESC(AGC_RES2,  RO, "Reserved area 2"),
    DW1000_SUB_REG_DESC(AGC_TUNE2, RW, "AGC Tuning register 2"),
    DW1000_SUB_REG_DESC(AGC_RES3,  RO, "Reserved area 3"),
    DW1000_SUB_REG_DESC(AGC_TUNE3, RW, "AGC Tuning register 3"),
    DW1000_SUB_REG_DESC(AGC_RES4,  RO, "Reserved area 4"),
    DW1000_SUB_REG_DESC(AGC_STAT1, RO, "AGC Status"),
    NULL
};

// Register file: 0x27 - Digital receiver configuration overview
static struct dw1000_reg dw1000_drx_conf_sub_regs[] =
{
    {.reg_file_id = DW1000_DRX_RES1,    .length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "DRX_RES1",    .desc = "Reserved area 1"},
    {.reg_file_id = DW1000_DRX_TUNE0b,  .length = 2,    .reg_file_type = DW1000_RW,  .mnemonic = "DRX_TUNE0b",  .desc = "Digital Tuning Register 0b"},
    {.reg_file_id = DW1000_DRX_TUNE1a,  .length = 2,    .reg_file_type = DW1000_RW,  .mnemonic = "DRX_TUNE1a",  .desc = "Digital Tuning Register 1a"},
    {.reg_file_id = DW1000_DRX_TUNE1b,  .length = 2,    .reg_file_type = DW1000_RW,  .mnemonic = "DRX_TUNE1b",  .desc = "Digital Tuning Register 1b"},
    {.reg_file_id = DW1000_DRX_TUNE2,   .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "DRX_TUNE2",   .desc = "Digital Tuning Register 2"},
    {.reg_file_id = DW1000_DRX_RES2,    .length = 20,   .reg_file_type = DW1000_RO,  .mnemonic = "DRX_RES2",    .desc = "Reserved area 2"},
    {.reg_file_id = DW1000_DRX_SFDTOC,  .length = 2,    .reg_file_type = DW1000_RW,  .mnemonic = "DRX_SFDTOC",  .desc = "SFD timeout"},
    {.reg_file_id = DW1000_DRX_RES3,    .length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "DRX_RES3",    .desc = "Reserved area 3"},
    {.reg_file_id = DW1000_DRX_PRETOC,  .length = 2,    .reg_file_type = DW1000_RW,  .mnemonic = "DRX_PRETOC",  .desc = "Preamble detection timeout"},
    {.reg_file_id = DW1000_DRX_TUNE4H,  .length = 2,    .reg_file_type = DW1000_RW,  .mnemonic = "DRX_TUNE4H",  .desc = "Digital Tuning Register 4H"},
    {.reg_file_id = DW1000_DRX_CAR_INT, .length = 3,    .reg_file_type = DW1000_RO,  .mnemonic = "DRX_CAR_INT", .desc = "Carrier Recovery Integrator Register"},
    {.reg_file_id = DW1000_RXPACC_NOSAT,.length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "RXPACC_NOSAT",.desc = "Unsaturated accumulated preamble symbols"},
    NULL
};

// Register file: 0x28 - Analog RF configuration block overview
static struct dw1000_reg dw1000_rf_conf_sub_regs[] =
{
    {.reg_file_id = DW1000_RF_RF_CONF,  .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "RF_CONF",    .desc = "RF Configuration Register"},
    {.reg_file_id = DW1000_RF_RES1,     .length = 7,    .reg_file_type = DW1000_RW,  .mnemonic = "RF_RES1",    .desc = "Reserved area 1"},
    {.reg_file_id = DW1000_RF_RXCTRLH,  .length = 1,    .reg_file_type = DW1000_RW,  .mnemonic = "RF_RXCTRLH", .desc = "Analog RX Control Register"},
    {.reg_file_id = DW1000_RF_TXCTRL,   .length = 3,    .reg_file_type = DW1000_RW,  .mnemonic = "RF_TXCTRL",  .desc = "Analog TX Control Register"},
    {.reg_file_id = DW1000_RF_RES2,     .length = 16,   .reg_file_type = DW1000_RW,  .mnemonic = "RF_RES2",    .desc = "Reserved area 2"},
    {.reg_file_id = DW1000_RF_STATUS,   .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "RF_STATUS",  .desc = "RF Status Register"},
    {.reg_file_id = DW1000_LDOTUNE,     .length = 5,    .reg_file_type = DW1000_RW,  .mnemonic = "LDOTUNE",    .desc = "LDO voltage tuning"},
    NULL
};

// Register file: 0x2B - Frequency synthesiser control block overview
static struct dw1000_reg dw1000_fs_ctrl_sub_regs[] =
{
    {.reg_file_id = DW1000_FS_RES1,     .length = 7,    .reg_file_type = DW1000_RW,  .mnemonic = "FS_RES1",    .desc = "Frequency synthesiser - Reserved area 1"},
    {.reg_file_id = DW1000_FS_PLLCFG,   .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "FS_PLLCFG",  .desc = "Frequency synthesiser - PLL configuration"},
    {.reg_file_id = DW1000_FS_PLLTUNE,  .length = 1,    .reg_file_type = DW1000_RW,  .mnemonic = "FS_PLLTUNE", .desc = "Frequency synthesiser - PLL Tuning"},
    {.reg_file_id = DW1000_FS_RES2,     .length = 2,    .reg_file_type = DW1000_RW,  .mnemonic = "FS_RES2",    .desc = "Frequency synthesiser - Reserved area 2"},
    {.reg_file_id = DW1000_FS_XTALT,    .length = 1,    .reg_file_type = DW1000_RW,  .mnemonic = "FS_XTALT",   .desc = "Frequency synthesiser - Crystal trim"},
    {.reg_file_id = DW1000_FS_RES3,     .length = 6,    .reg_file_type = DW1000_RW,  .mnemonic = "FS_RES3",    .desc = "Frequency synthesiser - Reserved area 3"},
    NULL
};

// Register file: 0x2C – Always-on system control overview
static struct dw1000_reg dw1000_aon_sub_regs[] =
{
    {.reg_file_id = DW1000_AON_WCFG,    .length = 2,    .reg_file_type = DW1000_RW,  .mnemonic = "AON_WCFG",   .desc = "AON Wakeup Configuration Register"},
    {.reg_file_id = DW1000_AON_CTRL,    .length = 1,    .reg_file_type = DW1000_RW,  .mnemonic = "AON_CTRL",   .desc = "AON Control Register"},
    {.reg_file_id = DW1000_AON_RDAT,    .length = 1,    .reg_file_type = DW1000_RW,  .mnemonic = "AON_RDAT",   .desc = "AON Direct Access Read Data Result"},
    {.reg_file_id = DW1000_AON_ADDR,    .length = 1,    .reg_file_type = DW1000_RW,  .mnemonic = "AON_ADDR",   .desc = "AON Direct Access Address"},
    {.reg_file_id = DW1000_AON_CFG0,    .length = 4,    .reg_file_type = DW1000_RW,  .mnemonic = "AON_CFG0",   .desc = "AON Configuration Register 0"},
    {.reg_file_id = DW1000_AON_CFG1,    .length = 2,    .reg_file_type = DW1000_RW,  .mnemonic = "AON_CFG1",   .desc = "AON Configuration Register 1"},
    NULL
};

// Register file: 0x2D – OTP Memory Interface overview
static struct dw1000_reg dw1000_otp_if_sub_regs[] =
{
    DW1000_SUB_REG_DESC(OTP_WDAT,  RW, "OTP Write Data"),
    DW1000_SUB_REG_DESC(OTP_ADDR,  RW, "OTP Address"),
    DW1000_SUB_REG_DESC(OTP_CTRL,  RW, "OTP Control"),
    DW1000_SUB_REG_DESC(OTP_STAT,  RW, "OTP Status"),
    DW1000_SUB_REG_DESC(OTP_RDAT,  RO, "OTP Read Data"),
    DW1000_SUB_REG_DESC(OTP_SRDAT, RW, "OTP SR Read Data"),
    DW1000_SUB_REG_DESC(OTP_SF,    RW, "OTP Special Function"),
    NULL
};

// Register file: 0x2E – Leading Edge Detection Interface overview
static struct dw1000_reg dw1000_lde_ctrl_sub_regs[] =
{
    DW1000_SUB_REG_DESC(LDE_THRESH, RO, "LDE Threshold report"),
    DW1000_SUB_REG_DESC(LDE_CFG1,   RW, "LDE Configuration Register 1"),
    DW1000_SUB_REG_DESC(LDE_PPINDX, RO, "LDE Peak Path Index"),
    DW1000_SUB_REG_DESC(LDE_PPAMPL, RO, "LDE Peak Path Amplitude"),
    DW1000_SUB_REG_DESC(LDE_RXANTD, RW, "LDE Receive Antenna Delay configuration"),
    DW1000_SUB_REG_DESC(LDE_CFG2,   RW, "LDE Configuration Register 2"),
    DW1000_SUB_REG_DESC(LDE_REPC,   RW, "LDE Replica Coefficient configuration"),
    NULL
};

// Register file: 0x2F – Digital Diagnostics Interface overview
static struct dw1000_reg dw1000_dig_diag_sub_regs[] =
{
    {.reg_file_id = DW1000_EVC_CTRL,    .length = 4,    .reg_file_type = DW1000_RO,  .mnemonic = "EVC_CTRL",   .desc = "Event Counter Control"},
    {.reg_file_id = DW1000_EVC_PHE,     .length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "EVC_PHE",    .desc = "PHR Error Counter"},
    {.reg_file_id = DW1000_EVC_RSE,     .length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "EVC_RSE",    .desc = "RSD Error Counter"},
    {.reg_file_id = DW1000_EVC_FCG,     .length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "EVC_FCG",    .desc = "Frame Check Sequence Good Counter"},
    {.reg_file_id = DW1000_EVC_FCE,     .length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "EVC_FCE",    .desc = "Frame Check Sequence Error Counter"},
    {.reg_file_id = DW1000_EVC_FFR,     .length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "EVC_FFR",    .desc = "Frame Filter Rejection Counter"},
    {.reg_file_id = DW1000_EVC_OVR,     .length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "EVC_OVR",    .desc = "RX Overrun Error Counter"},
    {.reg_file_id = DW1000_EVC_STO,     .length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "EVC_STO",    .desc = "SFD Timeout Counter"},
    {.reg_file_id = DW1000_EVC_PTO,     .length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "EVC_PTO",    .desc = "Preamble Timeout Counter"},
    {.reg_file_id = DW1000_EVC_FWTO,    .length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "EVC_FWTO",   .desc = "RX Frame Wait Timeout Counter"},
    {.reg_file_id = DW1000_EVC_TXFS,    .length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "EVC_TXFS",   .desc = "TX Frame Sent Counter"},
    {.reg_file_id = DW1000_EVC_HPW,     .length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "EVC_HPW",    .desc = "Half Period Warning Counter"},
    {.reg_file_id = DW1000_EVC_TPW,     .length = 2,    .reg_file_type = DW1000_RO,  .mnemonic = "EVC_TPW",    .desc = "Transmitter Power-Up Warning Counter"},
    {.reg_file_id = DW1000_EVC_RES1,    .length = 8,    .reg_file_type = DW1000_RW,  .mnemonic = "EVC_RES1",   .desc = "Digital Diagnostics Reserved Area 1"},
    {.reg_file_id = DW1000_EVC_TMC,     .length = 2,    .reg_file_type = DW1000_RW,  .mnemonic = "DIAG_TMC",   .desc = "Test Mode Control Register"},
    NULL
};

// Register file: 0x36 – Power Management and System Control overview
static struct dw1000_reg dw1000_pmsc_sub_regs[] =
{
    DW1000_SUB_REG_DESC(PMSC_CTRL0,  RW, "PMSC Control Register 0"),
    DW1000_SUB_REG_DESC(PMSC_CTRL1,  RW, "PMSC Control Register 1"),
    DW1000_SUB_REG_DESC(PMSC_RES1,   RW, "PMSC reserved area 1"),
    DW1000_SUB_REG_DESC(PMSC_SNOZT,  RW, "PMSC Snooze Time Register"),
    DW1000_SUB_REG_DESC(PMSC_RES2,   RW, "PMSC reserved area 2"),
    DW1000_SUB_REG_DESC(PMSC_TXFSEQ, RW, "PMSC fine grain TX sequencing control"),
    DW1000_SUB_REG_DESC(PMSC_LEDC,   RW, "PMSC LED Control Register"),
    NULL
};

uint8_t m_tx_buf[4096];
uint8_t m_rx_buf[4096];
uint8_t m_buf[256];

int dw1000_non_indexed_read(const struct spi_config *spi_cfg, uint8_t reg_file_id,
    void *buf, size_t len, const char *msg)
{
    int header_size = sizeof(union dw1000_tran_header1);
    int num_bytes = len + header_size;
    if ((buf == NULL) || (reg_file_id > 0x3F) || (len == 0) || (num_bytes > BUF_SIZE))
        goto err;

    union dw1000_tran_header1 header = {
        .rid = reg_file_id,
        // .si  = 0,
        .op  = dw1000_SPI_READ,
    };

    memset(m_rx_buf, 0, num_bytes);
    memset(m_tx_buf, 0, num_bytes);
    m_tx_buf[0] = header.value;

    // t9: Last SPICLK to SPICSn de-asserted, 40 ns
    // sleep_us(1);
    cs_select(spi_cfg->pin.csn);
    int num_written = spi_write_read_blocking(spi_cfg->spi, m_tx_buf, m_rx_buf, num_bytes);
    if (num_written != num_bytes) {
        cs_deselect(spi_cfg->pin.csn);
        printf("num_written (%d) != num_bytes (%d)\n", num_written, num_bytes);
        goto err;
    }
    cs_deselect(spi_cfg->pin.csn);
    memcpy(buf, m_rx_buf + header_size, len);

    if (msg)
        print_buf(buf, len, msg);

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

int dw1000_non_indexed_write(const struct spi_config *spi_cfg, uint8_t reg_file_id,
    void *buf, size_t len, const char *msg)
{
    int header_size = sizeof(union dw1000_tran_header1);
    int num_bytes = len + header_size;
    if ((buf == NULL) || (reg_file_id > 0x3F) || (len == 0) || (num_bytes > BUF_SIZE))
        goto err;

    union dw1000_tran_header1 header = {
        .rid = reg_file_id,
        // .si  = 0,
        .op  = dw1000_SPI_WRITE,
    };

    if (msg) {
        memset(m_buf, 0, len);
        if (dw1000_non_indexed_read(spi_cfg, reg_file_id, m_buf, len, msg))
            goto err;
        print_buf(buf, len, msg);
    }

    memset(m_tx_buf, 0, num_bytes);
    m_tx_buf[0] = header.value;
    memcpy(m_tx_buf + header_size, buf, len);

    // t9: Last SPICLK to SPICSn de-asserted, 40 ns
    // sleep_us(1);
    cs_select(spi_cfg->pin.csn);
    int num_written = spi_write_blocking(spi_cfg->spi, m_tx_buf, num_bytes);
    if (num_written != num_bytes) {
        cs_deselect(spi_cfg->pin.csn);
        printf("num_written (%d) != num_bytes (%d)\n", num_written, num_bytes);
        goto err;
    }
    cs_deselect(spi_cfg->pin.csn);

    if (msg) {
        memset(m_buf, 0, len);
        if (dw1000_non_indexed_read(spi_cfg, reg_file_id, m_buf, len, msg))
            goto err;
    }

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

int dw1000_short_indexed_read(const struct spi_config *spi_cfg, uint8_t reg_file_id,
    uint8_t sub_addr, void *buf, size_t len, const char *msg)
{
    int header_size = sizeof(union dw1000_tran_header2);
    int num_bytes = len + header_size;
    if ((buf == NULL) || (reg_file_id > 0x3F) || (sub_addr > 0x7F) || (len == 0) || (num_bytes > BUF_SIZE))
        goto err;

    union dw1000_tran_header2 header = {
        .rid      = reg_file_id,
        .si       = 1,
        .op       = dw1000_SPI_READ,
        .sub_addr = sub_addr,
        // .ext      = 0,
    };

    memset(m_rx_buf, 0, num_bytes);
    memset(m_tx_buf, 0, num_bytes);
    m_tx_buf[0] = header.value[0];
    m_tx_buf[1] = header.value[1];

    // t9: Last SPICLK to SPICSn de-asserted, 40 ns
    // sleep_us(1);
    cs_select(spi_cfg->pin.csn);
    int num_written = spi_write_read_blocking(spi_cfg->spi, m_tx_buf, m_rx_buf, num_bytes);
    if (num_written != num_bytes) {
        cs_deselect(spi_cfg->pin.csn);
        printf("num_written (%d) != num_bytes (%d)\n", num_written, num_bytes);
        goto err;
    }
    cs_deselect(spi_cfg->pin.csn);
    memcpy(buf, m_rx_buf + header_size, len);

    if (msg)
        print_buf(buf, len, msg);

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

int dw1000_short_indexed_write(const struct spi_config *spi_cfg, uint8_t reg_file_id,
    uint8_t sub_addr, void *buf, size_t len, const char *msg)
{
    int header_size = sizeof(union dw1000_tran_header2);
    int num_bytes = len + header_size;
    if ((buf == NULL) || (reg_file_id > 0x3F) || (sub_addr > 0x7F) || (len == 0) || (num_bytes > BUF_SIZE))
        goto err;

    union dw1000_tran_header2 header = {
        .rid      = reg_file_id,
        .si       = 1,
        .op       = dw1000_SPI_WRITE,
        .sub_addr = sub_addr,
        // .ext      = 0,
    };

    if (msg) {
        memset(m_buf, 0, len);
        if (dw1000_short_indexed_read(spi_cfg, reg_file_id, sub_addr, m_buf, len, msg))
            goto err;
        print_buf(buf, len, msg);
    }

    memset(m_tx_buf, 0, num_bytes);
    m_tx_buf[0] = header.value[0];
    m_tx_buf[1] = header.value[1];
    memcpy(m_tx_buf + header_size, buf, len);

    // t9: Last SPICLK to SPICSn de-asserted, 40 ns
    // sleep_us(1);
    cs_select(spi_cfg->pin.csn);
    int num_written = spi_write_blocking(spi_cfg->spi, m_tx_buf, num_bytes);
    if (num_written != num_bytes) {
        cs_deselect(spi_cfg->pin.csn);
        printf("num_written (%d) != num_bytes (%d)\n", num_written, num_bytes);
        goto err;
    }
    cs_deselect(spi_cfg->pin.csn);

    if (msg) {
        memset(m_buf, 0, len);
        if (dw1000_short_indexed_read(spi_cfg, reg_file_id, sub_addr, m_buf, len, msg))
            goto err;
    }

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

int dw1000_long_indexed_read(const struct spi_config *spi_cfg, uint8_t reg_file_id,
    uint16_t sub_addr, void *buf, size_t len, const char *msg)
{
    int header_size = sizeof(union dw1000_tran_header3);
    int num_bytes = len + header_size;
    if ((buf == NULL) || (reg_file_id > 0x3F) || (sub_addr > 0x7FFF) || (len == 0) || (num_bytes > BUF_SIZE))
        goto err;

    union dw1000_tran_header3 header = {
        .rid        = reg_file_id,
        .si         = 1,
        .op         = dw1000_SPI_READ,
        .sub_addr_l = sub_addr & 0x7F,
        .ext        = 1,
        .sub_addr_h = sub_addr >> 7,
    };

    memset(m_rx_buf, 0, num_bytes);
    memset(m_tx_buf, 0, num_bytes);
    m_tx_buf[0] = header.value[0];
    m_tx_buf[1] = header.value[1];
    m_tx_buf[2] = header.value[2];

    // t9: Last SPICLK to SPICSn de-asserted, 40 ns
    // sleep_us(1);
    cs_select(spi_cfg->pin.csn);
    int num_written = spi_write_read_blocking(spi_cfg->spi, m_tx_buf, m_rx_buf, num_bytes);
    if (num_written != num_bytes) {
        cs_deselect(spi_cfg->pin.csn);
        printf("num_written (%d) != num_bytes (%d)\n", num_written, num_bytes);
        goto err;
    }
    cs_deselect(spi_cfg->pin.csn);
    memcpy(buf, m_rx_buf + header_size, len);

    if (msg)
        print_buf(buf, len, msg);

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

int dw1000_long_indexed_write(const struct spi_config *spi_cfg, uint8_t reg_file_id,
    uint16_t sub_addr, void *buf, size_t len, const char *msg)
{
    int header_size = sizeof(union dw1000_tran_header3);
    int num_bytes = len + header_size;
    if ((buf == NULL) || (reg_file_id > 0x3F) || (sub_addr > 0x7FFF) || (len == 0) || (num_bytes > BUF_SIZE))
        goto err;

    union dw1000_tran_header3 header = {
        .rid        = reg_file_id,
        .si         = 1,
        .op         = dw1000_SPI_WRITE,
        .sub_addr_l = sub_addr & 0x7F,
        .ext        = 1,
        .sub_addr_h = sub_addr >> 7,
    };

    if (msg) {
        memset(m_buf, 0, len);
        if (dw1000_long_indexed_read(spi_cfg, reg_file_id, sub_addr, m_buf, len, msg))
            goto err;
        print_buf(buf, len, msg);
    }

    memset(m_tx_buf, 0, num_bytes);
    m_tx_buf[0] = header.value[0];
    m_tx_buf[1] = header.value[1];
    m_tx_buf[2] = header.value[2];
    memcpy(m_tx_buf + header_size, buf, len);

    // t9: Last SPICLK to SPICSn de-asserted, 40 ns
    // sleep_us(1);
    cs_select(spi_cfg->pin.csn);
    int num_written = spi_write_blocking(spi_cfg->spi, m_tx_buf, num_bytes);
    if (num_written != num_bytes) {
        cs_deselect(spi_cfg->pin.csn);
        printf("num_written (%d) != num_bytes (%d)\n", num_written, num_bytes);
        goto err;
    }
    cs_deselect(spi_cfg->pin.csn);

    if (msg) {
        memset(m_buf, 0, len);
        if (dw1000_long_indexed_read(spi_cfg, reg_file_id, sub_addr, m_buf, len, msg))
            goto err;
    }

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

static const char *prf[] = {
    [DW1000_PRF_4MHZ]  = "4 MHz",
    [DW1000_PRF_16MHZ] = "16 MHz",
    [DW1000_PRF_64MHZ] = "64 MHz",
    [DW1000_PRF_RSVD]  = "Reserved",
};

int dw1000_dump_all_regs(const struct spi_config *spi_cfg)
{
    uint8_t tx_buf[4096], rx_buf[4096];
    memset(tx_buf, 0, sizeof(tx_buf));
    memset(rx_buf, 0, sizeof(rx_buf));

    for (const struct dw1000_reg *reg = dw1000_regs; reg->mnemonic != NULL; reg++) {
        if ((reg->length > 64) || (reg->length == 0 && reg->reg_file_id != DW1000_LDE_CTRL))
            continue;

        // if (reg->reg_file_id != DW1000_SYS_STATUS)
        //     continue;

        if (reg->reg_file_id != DW1000_LDE_CTRL) {
            memset(rx_buf, 0, reg->length);
            if (dw1000_non_indexed_read(spi_cfg, reg->reg_file_id, rx_buf, reg->length, NULL))
                goto err;
            print_buf(rx_buf, reg->length, "Register file: 0x%02X - %s\n", reg->reg_file_id, reg->desc);
        }

        switch (reg->reg_file_id) {
        case DW1000_DEV_ID:
            union DW1000_REG_DEV_ID *dev_id = (void *)rx_buf;
            printf("dev_id->value               : %08x\n", dev_id->value);
            printf("dev_id->rev                 : %x\n", dev_id->rev);
            printf("dev_id->ver                 : %x\n", dev_id->ver);
            printf("dev_id->model               : %x\n", dev_id->model);
            printf("dev_id->ridtag              : %x\n", dev_id->ridtag);

            memset(rx_buf, 0, reg->length);
            if (dw1000_short_indexed_read(spi_cfg, reg->reg_file_id, 2, rx_buf, 2, NULL))
                goto err;
            print_buf(rx_buf, 2, NULL);
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
            if (dw1000_non_indexed_write(spi_cfg, reg->reg_file_id, tx_buf, reg->length, NULL))
                goto err;

            memset(rx_buf, 0, reg->length);
            if (dw1000_non_indexed_read(spi_cfg, reg->reg_file_id, rx_buf, reg->length, NULL))
                goto err;

            print_buf(rx_buf, reg->length, "%s (%02xh)\n", reg->desc, reg->reg_file_id);
            break;

        case DW1000_PANADR:
            union DW1000_REG_PANADR *panadr = (void *)rx_buf;
            printf("panadr->short_addr          : %x\n", panadr->short_addr);
            printf("panadr->pan_id              : %x\n", panadr->pan_id);
            break;

        case DW1000_SYS_CFG:
            union DW1000_REG_SYS_CFG *sys_cfg = (void *)rx_buf;
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
            printf("sys_cfg->phr_mode           : %x\n", sys_cfg->phr_mode);
            printf("sys_cfg->dis_stxp           : %x\n", sys_cfg->dis_stxp);
            printf("sys_cfg->rxm110k            : %x\n", sys_cfg->rxm110k);
            printf("sys_cfg->rxwtoe             : %x\n", sys_cfg->rxwtoe);
            printf("sys_cfg->rxautr             : %x\n", sys_cfg->rxautr);
            printf("sys_cfg->autoack            : %x\n", sys_cfg->autoack);
            printf("sys_cfg->aackpend           : %x\n", sys_cfg->aackpend);
            break;

        case DW1000_TX_FCTRL:
            union DW1000_REG_TX_FCTRL *tx_fctrl = (void *)rx_buf;
            printf("Transmit Frame Length       : %d bytes\n", tx_fctrl->ofs_00.tflen);
            printf("tx_fctrl->ofs_00.tfle       : %x\n", tx_fctrl->ofs_00.tfle);
            printf("tx_fctrl->ofs_00.r          : %x\n", tx_fctrl->ofs_00.r);
            uint16_t txbr[4] = {
                [0]  = 110,
                [1]  = 850,
                [2]  = 6800,
            };
            printf("PRF                         : %d kbps\n", txbr[tx_fctrl->ofs_00.txbr]);
            printf("tx_fctrl->ofs_00.tr         : %x\n", tx_fctrl->ofs_00.tr);
            printf("PRF                         : %d (%s)\n", tx_fctrl->ofs_00.txprf, prf[tx_fctrl->ofs_00.txprf]);
            uint16_t txpsr[16] = {
                [0x1] = 64,
                [0x2] = 1024,
                [0x3] = 4096,
                [0x5] = 128,
                [0x9] = 256,
                [0xd] = 512,
                [0x6] = 1536,
                [0xa] = 2048,
            };
            printf("Preamble Length             : %d bytes\n", txpsr[(tx_fctrl->ofs_00.pe << 2) | tx_fctrl->ofs_00.txpsr]);
            printf("tx_fctrl->ofs_00.txboffs    : %x\n", tx_fctrl->ofs_00.txboffs);
            printf("tx_fctrl->ofs_04.ifsdelay   : %x\n", tx_fctrl->ofs_04.ifsdelay);
            break;

        case DW1000_SYS_STATUS:
            union DW1000_REG_SYS_STATUS *sys_status = (void *)rx_buf;
            printf("sys_status->ofs_00.irqs     : %d\n", sys_status->ofs_00.irqs);
            printf("sys_status->ofs_00.cplock   : %d\n", sys_status->ofs_00.cplock);
            printf("sys_status->ofs_00.esyncr   : %d\n", sys_status->ofs_00.esyncr);
            printf("sys_status->ofs_00.aat      : %d\n", sys_status->ofs_00.aat);
            printf("sys_status->ofs_00.txfrb    : %d\n", sys_status->ofs_00.txfrb);
            printf("sys_status->ofs_00.txprs    : %d\n", sys_status->ofs_00.txprs);
            printf("sys_status->ofs_00.txphs    : %d\n", sys_status->ofs_00.txphs);
            printf("sys_status->ofs_00.txfrs    : %d\n", sys_status->ofs_00.txfrs);
            //
            printf("sys_status->ofs_00.rxdfr    : %d\n", sys_status->ofs_00.rxdfr);
            printf("sys_status->ofs_00.rxsfdd   : %d\n", sys_status->ofs_00.rxsfdd);
            printf("sys_status->ofs_00.ldedone  : %d\n", sys_status->ofs_00.ldedone);
            printf("sys_status->ofs_00.rxphd    : %d\n", sys_status->ofs_00.rxphd);
            printf("sys_status->ofs_00.rxphe    : %d\n", sys_status->ofs_00.rxphe);
            printf("sys_status->ofs_00.rxdfr    : %d\n", sys_status->ofs_00.rxdfr);
            printf("sys_status->ofs_00.rxfcg    : %d\n", sys_status->ofs_00.rxfcg);
            printf("sys_status->ofs_00.rxfce    : %d\n", sys_status->ofs_00.rxfce);
            //
            printf("sys_status->ofs_00.rxrfsl   : %d\n", sys_status->ofs_00.rxrfsl);
            printf("sys_status->ofs_00.rxrfto   : %d\n", sys_status->ofs_00.rxrfto);
            printf("sys_status->ofs_00.ldeerr   : %d\n", sys_status->ofs_00.ldeerr);
            printf("sys_status->ofs_00.rsvd     : %d\n", sys_status->ofs_00.rsvd);
            printf("sys_status->ofs_00.rxovrr   : %d\n", sys_status->ofs_00.rxovrr);
            printf("sys_status->ofs_00.rxpto    : %d\n", sys_status->ofs_00.rxpto);
            printf("sys_status->ofs_00.gpioirq  : %d\n", sys_status->ofs_00.gpioirq);
            printf("sys_status->ofs_00.slp2init : %d\n", sys_status->ofs_00.slp2init);
            //
            printf("sys_status->ofs_00.rfpll_ll : %d\n", sys_status->ofs_00.rfpll_ll);
            printf("sys_status->ofs_00.clkpll_ll: %d\n", sys_status->ofs_00.clkpll_ll);
            printf("sys_status->ofs_00.rxsfdto  : %d\n", sys_status->ofs_00.rxsfdto);
            printf("sys_status->ofs_00.hpdwarn  : %d\n", sys_status->ofs_00.hpdwarn);
            printf("sys_status->ofs_00.txberr   : %d\n", sys_status->ofs_00.txberr);
            printf("sys_status->ofs_00.affrej   : %d\n", sys_status->ofs_00.affrej);
            printf("sys_status->ofs_00.hsrbp    : %d\n", sys_status->ofs_00.hsrbp);
            printf("sys_status->ofs_00.icrbp    : %d\n", sys_status->ofs_00.icrbp);
            //
            printf("sys_status->ofs_04.affrej   : %d\n", sys_status->ofs_04.rxrscs);
            printf("sys_status->ofs_04.hsrbp    : %d\n", sys_status->ofs_04.rxprej);
            printf("sys_status->ofs_04.icrbp    : %d\n", sys_status->ofs_04.txpute);
            printf("sys_status->ofs_04.rsvd     : %d\n", sys_status->ofs_04.rsvd);
            sys_status->ofs_00.value = 0xFFFFFFFF;
            sys_status->ofs_04.value = 0xFF;;
            if (dw1000_non_indexed_write(spi_cfg, reg->reg_file_id, sys_status, sizeof(*sys_status), NULL))
                goto err;

            break;

        case DW1000_CHAN_CTRL:
            union DW1000_REG_CHAN_CTRL *chan_ctrl = (void *)rx_buf;
            printf("chan_ctrl->tx_chan          : %d\n", chan_ctrl->tx_chan);
            printf("chan_ctrl->rx_chan          : %d\n", chan_ctrl->rx_chan);
            printf("chan_ctrl->rsvd             : %d\n", chan_ctrl->rsvd);
            printf("chan_ctrl->dwsfd            : %d\n", chan_ctrl->dwsfd);
            printf("chan_ctrl->rxprf            : %d (%s)\n", chan_ctrl->rxprf, prf[chan_ctrl->rxprf]);
            printf("chan_ctrl->tnssfd           : %d\n", chan_ctrl->tnssfd);
            printf("chan_ctrl->rnssfd           : %d\n", chan_ctrl->rnssfd);
            const char *pcode[] = {
                [1 ... 8]   = "For 16 MHz PRF",
                [9 ... 12]  = "For 64 MHz PRF",
                [17 ... 20] = "For 64 MHz PRF",
                [13 ... 16] = "For 64 MHz PRF (DPS)",
                [21 ... 24] = "For 64 MHz PRF (DPS)",
            };
            printf("chan_ctrl->tx_pcode         : %d (%s)\n", chan_ctrl->tx_pcode, pcode[chan_ctrl->tx_pcode]);
            printf("chan_ctrl->rx_pcode         : %d (%s)\n", chan_ctrl->rx_pcode, pcode[chan_ctrl->rx_pcode]);
            break;

        case DW1000_AGC_CTRL:
            for (struct dw1000_reg *sub_reg = dw1000_agc_ctrl_sub_regs; sub_reg->mnemonic != NULL; sub_reg++) {
                if ((sub_reg->length > 64) || (sub_reg->length == 0))
                    continue;

                memset(rx_buf, 0, sub_reg->length);
                if (dw1000_short_indexed_read(spi_cfg, reg->reg_file_id, sub_reg->reg_file_id, rx_buf, sub_reg->length, NULL))
                    goto err;

                print_buf(rx_buf, sub_reg->length, "Sub-Register 0x%02X:%02X - %s\n", reg->reg_file_id, sub_reg->reg_file_id, sub_reg->desc);
            }
            break;

        case DW1000_DRX_CONF:
            for (struct dw1000_reg *sub_reg = dw1000_drx_conf_sub_regs; sub_reg->mnemonic != NULL; sub_reg++) {
                if ((sub_reg->length > 64) || (sub_reg->length == 0))
                    continue;

                memset(rx_buf, 0, sub_reg->length);
                if (dw1000_short_indexed_read(spi_cfg, reg->reg_file_id, sub_reg->reg_file_id, rx_buf, sub_reg->length, NULL))
                    goto err;

                print_buf(rx_buf, sub_reg->length, "Sub-Register 0x%02X:%02X - %s\n", reg->reg_file_id, sub_reg->reg_file_id, sub_reg->desc);
            }
            break;

        case DW1000_RF_CONF:
            for (struct dw1000_reg *sub_reg = dw1000_rf_conf_sub_regs; sub_reg->mnemonic != NULL; sub_reg++) {
                if ((sub_reg->length > 64) || (sub_reg->length == 0))
                    continue;

                memset(rx_buf, 0, sub_reg->length);
                if (dw1000_short_indexed_read(spi_cfg, reg->reg_file_id, sub_reg->reg_file_id, rx_buf, sub_reg->length, NULL))
                    goto err;

                print_buf(rx_buf, sub_reg->length, "Sub-Register 0x%02X:%02X - %s\n", reg->reg_file_id, sub_reg->reg_file_id, sub_reg->desc);
            }
            break;

        case DW1000_FS_CTRL:
            for (struct dw1000_reg *sub_reg = dw1000_fs_ctrl_sub_regs; sub_reg->mnemonic != NULL; sub_reg++) {
                if ((sub_reg->length > 64) || (sub_reg->length == 0))
                    continue;

                memset(rx_buf, 0, sub_reg->length);
                if (dw1000_short_indexed_read(spi_cfg, reg->reg_file_id, sub_reg->reg_file_id, rx_buf, sub_reg->length, NULL))
                    goto err;

                print_buf(rx_buf, sub_reg->length, "Sub-Register 0x%02X:%02X - %s\n", reg->reg_file_id, sub_reg->reg_file_id, sub_reg->desc);
            }
            break;

        case DW1000_AON:
            for (struct dw1000_reg *sub_reg = dw1000_aon_sub_regs; sub_reg->mnemonic != NULL; sub_reg++) {
                if ((sub_reg->length > 64) || (sub_reg->length == 0))
                    continue;

                memset(rx_buf, 0, sub_reg->length);
                if (dw1000_short_indexed_read(spi_cfg, reg->reg_file_id, sub_reg->reg_file_id, rx_buf, sub_reg->length, NULL))
                    goto err;

                print_buf(rx_buf, sub_reg->length, "Sub-Register 0x%02X:%02X - %s\n", reg->reg_file_id, sub_reg->reg_file_id, sub_reg->desc);

                switch (sub_reg->reg_file_id) {
                case DW1000_AON_CFG0:
                    union DW1000_SUB_REG_AON_CFG0 *aon_cfg0 = (void *)rx_buf;
                    printf("aon_cfg0->sleep_en          : %d\n", aon_cfg0->sleep_en);
                    printf("aon_cfg0->wake_pin          : %d\n", aon_cfg0->wake_pin);
                    printf("aon_cfg0->wake_spi          : %d\n", aon_cfg0->wake_spi);
                    printf("aon_cfg0->wake_cnt          : %d\n", aon_cfg0->wake_cnt);
                    printf("aon_cfg0->lpdiv_en          : %d\n", aon_cfg0->lpdiv_en);
                    printf("aon_cfg0->lpclkdiva         : %d\n", aon_cfg0->lpclkdiva);
                    printf("aon_cfg0->sleep_tim         : %d\n", aon_cfg0->sleep_tim);
                    break;
                }
            }
            break;

        case DW1000_OTP_IF:
            for (struct dw1000_reg *sub_reg = dw1000_otp_if_sub_regs; sub_reg->mnemonic != NULL; sub_reg++) {
                if ((sub_reg->length > 64) || (sub_reg->length == 0))
                    continue;

                memset(rx_buf, 0, sub_reg->length);
                if (dw1000_short_indexed_read(spi_cfg, reg->reg_file_id, sub_reg->reg_file_id, rx_buf, sub_reg->length, NULL))
                    goto err;

                print_buf(rx_buf, sub_reg->length, "Sub-Register 0x%02X:%02X - %s\n", reg->reg_file_id, sub_reg->reg_file_id, sub_reg->desc);
            }
            break;

        case DW1000_LDE_CTRL:
            for (struct dw1000_reg *sub_reg = dw1000_lde_ctrl_sub_regs; sub_reg->mnemonic != NULL; sub_reg++) {
                if ((sub_reg->length > 64) || (sub_reg->length == 0))
                    continue;

                memset(rx_buf, 0, sub_reg->length);
                if (dw1000_long_indexed_read(spi_cfg, reg->reg_file_id, sub_reg->reg_file_id, rx_buf, sub_reg->length, NULL))
                    goto err;

                print_buf(rx_buf, sub_reg->length, "Sub-Register 0x%02X:%02X - %s\n", reg->reg_file_id, sub_reg->reg_file_id, sub_reg->desc);
            }
            break;

        case DW1000_DIG_DIAG:
            for (struct dw1000_reg *sub_reg = dw1000_dig_diag_sub_regs; sub_reg->mnemonic != NULL; sub_reg++) {
                if ((sub_reg->length > 64) || (sub_reg->length == 0))
                    continue;

                memset(rx_buf, 0, sub_reg->length);
                if (dw1000_short_indexed_read(spi_cfg, reg->reg_file_id, sub_reg->reg_file_id, rx_buf, sub_reg->length, NULL))
                    goto err;

                print_buf(rx_buf, sub_reg->length, "Sub-Register 0x%02X:%02X - %s\n", reg->reg_file_id, sub_reg->reg_file_id, sub_reg->desc);

                switch (sub_reg->reg_file_id) {
                case DW1000_EVC_CTRL:
                    union DW1000_SUB_REG_EVC_CTRL *evc_ctrl = (void *)rx_buf;
                    printf("evc_ctrl->evc_en            : %d\n", evc_ctrl->evc_en);
                    printf("evc_ctrl->evc_clr           : %d\n", evc_ctrl->evc_clr);
                    break;

                case DW1000_DIG_DIAG:
                    union DW1000_SUB_REG_DIAG_TMC *diag_tmc = (void *)rx_buf;
                    printf("diag_tmc->tx_pstm           : %d\n", diag_tmc->tx_pstm);
                    break;
                }
            }
            break;

        case DW1000_PMSC:
            for (struct dw1000_reg *sub_reg = dw1000_pmsc_sub_regs; sub_reg->mnemonic != NULL; sub_reg++) {
                if ((sub_reg->length > 64) || (sub_reg->length == 0))
                    continue;

                memset(rx_buf, 0, sub_reg->length);
                if (dw1000_short_indexed_read(spi_cfg, reg->reg_file_id, sub_reg->reg_file_id, rx_buf, sub_reg->length, NULL))
                    goto err;

                print_buf(rx_buf, sub_reg->length, "Sub-Register 0x%02X:%02X - %s\n", reg->reg_file_id, sub_reg->reg_file_id, sub_reg->desc);
            }
            break;
        };
    }

    return 0;
err:
    return -1;
}

/**
 * @brief Perform a hardware reset on the DW1000 transceiver.
 *
 * This function asserts and de-asserts the DW1000 RSTn pin to trigger a full
 * hardware reset of the device. After the reset sequence, it writes to the
 * EC_CTRL sub-register to enable the PLL lock-detect tuning bit (`pllldt`),
 * which improves the accuracy of clock-PLL lock detection during subsequent
 * initialization.
 *
 * Reset sequence:
 * 1. Drive RSTn low for 1 ms to assert reset.
 * 2. Drive RSTn high for 1 ms to release reset.
 * 3. Write EC_CTRL with `pllldt = 1` to enable PLL lock-detect tuning.
 *
 * @retval 0  Hardware reset and EC_CTRL configuration succeeded.
 * @retval -1 SPI write failed or other internal error occurred.
 *
 * @note This function should be called before any register access that depends
 *       on a stable system clock. It is typically invoked at the start of
 *       @ref dw1000_init() before waiting for the PLL to lock.
 */
int dw1000_hard_reset()
{
    printf("RSTn S\n");
    gpio_put(RSTn_PIN, 0);
    sleep_ms(1);
    gpio_put(RSTn_PIN, 1);
    sleep_ms(1);
    printf("RSTn E\n");

    // Enable Clock PLL lock detect tune.
    const struct spi_config *spi_cfg = &m_dw1000_ctx.spi_cfg;
    union DW1000_SUB_REG_EC_CTRL *ec_ctrl = &m_dw1000_ctx.ec_ctrl;
    ec_ctrl->pllldt = 1;
    if (dw1000_short_indexed_write(spi_cfg, DW1000_EXT_SYNC, DW1000_EC_CTRL, ec_ctrl, sizeof(*ec_ctrl), NULL))
        goto err;

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

/**
 * @brief Clear all status bits by writing all 1s
 */
int dw1000_clear_sys_status(const struct spi_config *spi_cfg)
{
    // Clear the interrupt status
    union DW1000_REG_SYS_STATUS sys_status = {.ofs_00.value = UINT32_MAX, .ofs_04.value = UINT8_MAX};
    if (dw1000_non_indexed_write(spi_cfg, DW1000_SYS_STATUS, &sys_status, sizeof(sys_status), NULL)) {
        printf("%s failed\n", __func__);
        return -1;
    }

    return 0;
}

int dw1000_clear_sys_mask(const struct spi_config *spi_cfg)
{
    union DW1000_REG_SYS_MASK sys_mask = {.value = UINT32_MAX};
    if (dw1000_non_indexed_write(spi_cfg, DW1000_SYS_MASK, &sys_mask, sizeof(sys_mask), NULL)) {
        printf("%s failed\n", __func__);
        return -1;
    }

    return 0;
}

int dw1000_clear_sys_status_ofs_00(const struct spi_config *spi_cfg)
{
#if (CONFIG_DW1000_SYS_STS_DEBUG)
    uint8_t temp[5];
    if (dw1000_non_indexed_read(spi_cfg, DW1000_SYS_STATUS, temp, sizeof(temp), NULL)) {
        printf("%s failed\n", __func__);
        return -1;
    }
    print_buf(temp, sizeof(temp), "sys_status 1\n");
#endif

    // Clear the interrupt status
    union DW1000_REG_SYS_STATUS sys_status = {.ofs_00.value = UINT32_MAX};
    if (dw1000_short_indexed_write(spi_cfg, DW1000_SYS_STATUS, offsetof(union DW1000_REG_SYS_STATUS, ofs_00), &sys_status.ofs_00, sizeof(sys_status.ofs_00), NULL)) {
        printf("%s failed\n", __func__);
        return -1;
    }

#if (CONFIG_DW1000_SYS_STS_DEBUG)
    if (dw1000_non_indexed_read(spi_cfg, DW1000_SYS_STATUS, temp, sizeof(temp), NULL)) {
        printf("%s failed\n", __func__);
        return -1;
    }
    print_buf(temp, sizeof(temp), "sys_status 2\n");
#endif

    return 0;
}

/**
 * @brief Clear SYS_STATUS (0x0F:00..03) bits via W1C mask.
 */
int dw1000_clear_sys_status_ofs_00_by_mask(const struct spi_config *spi_cfg, uint32_t mask)
{
#if (CONFIG_DW1000_SYS_STS_DEBUG)
    uint8_t temp[5];
    if (dw1000_non_indexed_read(spi_cfg, DW1000_SYS_STATUS, temp, sizeof(temp), NULL)) {
        printf("%s failed\n", __func__);
        return -1;
    }
    print_buf(temp, sizeof(temp), "sys_status 3\n");
#endif

    // Clear the interrupt status
    if (dw1000_short_indexed_write(spi_cfg, DW1000_SYS_STATUS, offsetof(union DW1000_REG_SYS_STATUS, ofs_00), &mask, sizeof(mask), NULL)) {
        printf("%s failed\n", __func__);
        return -1;
    }

#if (CONFIG_DW1000_SYS_STS_DEBUG)
    if (dw1000_non_indexed_read(spi_cfg, DW1000_SYS_STATUS, temp, sizeof(temp), NULL)) {
        printf("%s failed\n", __func__);
        return -1;
    }
    print_buf(temp, sizeof(temp), "sys_status 4\n");
#endif

    return 0;
}

_Static_assert(offsetof(union DW1000_REG_SYS_STATUS, ofs_00) == 0, "offsetof(union DW1000_REG_SYS_STATUS, ofs_00) != 0");

int dw1000_clear_sys_status_ofs_04(const struct spi_config *spi_cfg)
{
    // Clear the interrupt status
    union DW1000_REG_SYS_STATUS sys_status = {.ofs_04.value = UINT8_MAX};
    if (dw1000_short_indexed_write(spi_cfg, DW1000_SYS_STATUS, offsetof(union DW1000_REG_SYS_STATUS, ofs_04), &sys_status.ofs_04, sizeof(sys_status.ofs_04), NULL)) {
        printf("%s failed\n", __func__);
        return -1;
    }

    return 0;
}

_Static_assert(offsetof(union DW1000_REG_SYS_STATUS, ofs_04) == 4, "offsetof(union DW1000_REG_SYS_STATUS, ofs_04) != 4");

/**
 * @brief @brief Clear SYS_STATUS (0x0F:04) bits via W1C mask.
 */
int dw1000_clear_sys_status_ofs_04_by_mask(const struct spi_config *spi_cfg, uint8_t mask)
{
    // Clear the interrupt status
    if (dw1000_short_indexed_write(spi_cfg, DW1000_SYS_STATUS, offsetof(union DW1000_REG_SYS_STATUS, ofs_04), &mask, sizeof(mask), NULL)) {
        printf("%s failed\n", __func__);
        return -1;
    }

    return 0;
}

/**
 * @brief Clear all status bits by writing all 1s
 */
int dw1000_clear_sys_status_check(const struct spi_config *spi_cfg)
{
    // Clear the interrupt status
    union DW1000_REG_SYS_STATUS sys_status = {.ofs_00.value = UINT32_MAX, .ofs_04.value = UINT8_MAX};
    if (dw1000_non_indexed_write(spi_cfg, DW1000_SYS_STATUS, &sys_status, sizeof(sys_status), NULL))
        goto err;

    sleep_ms(1);

    if (dw1000_non_indexed_read(spi_cfg, DW1000_SYS_STATUS, &sys_status, sizeof(sys_status), NULL))
        goto err;

    if (sys_status.ofs_00.value || sys_status.ofs_04.value)
        goto err;

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

/**
 * @brief Wait until the DW1000 PLL (Phase-Locked Loop) is locked.
 *
 * This function performs a hardware reset and continuously polls both the
 * SYS_STATUS and RF_STATUS registers to check whether the digital and RF
 * PLLs have successfully locked. If either lock signal is missing after
 * multiple retries, the function will perform another reset and continue
 * checking until the lock is achieved or the retry limit is reached.
 *
 * @retval 0  PLL successfully locked.
 * @retval -1 Failure occurred (hardware reset or SPI access error, or PLL did not lock).
 */
int dw1000_wait_pll_lock()
{
    // Wait PLL Lock
    for (int i = 0; ; i++) {
        if (i == 100) {
            printf("Clock PLL lock failed.\n");
            return -1;
        }
        const struct spi_config *spi_cfg = &m_dw1000_ctx.spi_cfg;
        union DW1000_REG_SYS_STATUS sys_status;
        union DW1000_SUB_REG_RF_STATUS rf_status;
        if (dw1000_non_indexed_read(spi_cfg, DW1000_SYS_STATUS, &sys_status, sizeof(sys_status), NULL))
            goto err;

        if (dw1000_short_indexed_read(spi_cfg, DW1000_RF_CONF, DW1000_RF_STATUS, &rf_status, sizeof(rf_status), NULL))
            goto err;

        printf("sys_status:%02x_%08x,cplock:%d\n", sys_status.ofs_04.value, sys_status.ofs_00.value, sys_status.ofs_00.cplock);
        printf("rf_status:%08x,cplllock:%d\n", rf_status.value, rf_status.cplllock);

        if (!rf_status.cplllock) {
            printf("[WARN] PLL not locked (attempt %d). Reinitializing...\n", i + 1);
            if (dw1000_hard_reset())
                goto err;
        } else {
            printf("PLL locked successfully.\n");
            if (dw1000_clear_sys_status(spi_cfg))
                goto err;
            break;
        }
    }

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

/**
 * TODO: LDOTUNE
 * TODO: External Synchronisation
 * TODO: IC Calibration – Crystal Oscillator Trim
 */
int dw1000_init()
{
    printf("%s\n", __func__);

    // Perform initial hardware reset before checking PLL status
    if (dw1000_hard_reset())
        goto err;

    if (dw1000_wait_pll_lock())
        goto err;

    /* *************************************************************************
     *                           System Configuration
     * ************************************************************************/

    /**
     * Much of the system configuration is configured in the SYS_CFG register,
     * please see section Register file: 0x04 – System Configuration for a full
     * description of the register contents and defaults.
     *
     * By default, interrupt polarity is active high and all interrupts are
     * disabled, see the SYS_CFG register for interrupt polarity
     *
     * Other SYS_CFG register settings such as Automatic Receiver Re-Enable
     * (RXAUTR) and MAC functions such as frame filtering (FFEN), double
     * buffering (DIS_DRXB) and automatic acknowledgement (AUTOACK) are all off
     * by default. Automatic CRC generation is on and the CRC LFSR is
     * initialized to 0’s (FCS_INIT2F).
     */
    union DW1000_REG_SYS_CFG *sys_cfg = &m_dw1000_ctx.sys_cfg;
    sys_cfg->hirq_pol = DW1000_HIRQ_POL_ACTIVE_HIGH;
    sys_cfg->dis_drxb = true;
    // sys_cfg->rxm110k  = true;
    sys_cfg->rxm110k  = false;
    sys_cfg->rxwtoe = true;
#if (CONFIG_DW1000_AUTO_RX)
    sys_cfg->rxautr   = true;
#endif
    sys_cfg->dis_stxp = (sys_cfg->rxm110k ? true : false);

    printf("Host interrupt polarity          : %s\n", (sys_cfg->hirq_pol ? "true" : "false"));
    printf("Disable Double RX Buffer         : %s\n", (sys_cfg->dis_drxb ? "true" : "false"));
    printf("Disable Smart TX Power control   : %s\n", (sys_cfg->dis_stxp ? "true" : "false"));
    printf("Receiver Mode 110 kbps data rate : %s\n", (sys_cfg->rxm110k  ? "true" : "false"));
    printf("Receiver Auto-Re-enable          : %s\n", (sys_cfg->rxautr   ? "true" : "false"));

    const struct spi_config *spi_cfg = &m_dw1000_ctx.spi_cfg;
    if (dw1000_non_indexed_write(spi_cfg, DW1000_SYS_CFG, sys_cfg, sizeof(*sys_cfg), "sys_cfg: "))
        goto err;

    /**
     * frame wait timeout (see SYS_CFG register bit RXWTOE and Register file:
     * 0x0C – Receive Frame Wait Timeout Period)
     */
    // if (sys_cfg->rxwtoe) {
        union DW1000_REG_RX_FWTO *rx_fwto = &m_dw1000_ctx.rx_fwto;
        rx_fwto->rxfwto = UINT16_MAX;
        if (dw1000_non_indexed_write(spi_cfg, DW1000_RX_FWTO, rx_fwto, sizeof(*rx_fwto), "rx_fwto: "))
            goto err;
    // }

    union DW1000_SUB_REG_GPIO_MODE *gpio_mode = &m_dw1000_ctx.gpio_mode;
    gpio_mode->value = 0;
    if (dw1000_short_indexed_write(spi_cfg, DW1000_GPIO_CTRL, DW1000_GPIO_MODE, gpio_mode, sizeof(*gpio_mode), "gpio_mode: "))
        goto err;

    /**
     * Sniff mode is off, see Register file: 0x1D – SNIFF Mode for details
     */
    union DW1000_REG_RX_SNIFF *rx_sniff = &m_dw1000_ctx.rx_sniff;
    rx_sniff->value = 0;
    if (dw1000_non_indexed_write(spi_cfg, DW1000_RX_SNIFF, rx_sniff, sizeof(*rx_sniff), "rx_sniff: "))
        goto err;

    union DW1000_REG_PMSC *pmsc = &m_dw1000_ctx.pmsc;
    if (dw1000_short_indexed_read(spi_cfg, DW1000_PMSC, DW1000_PMSC_CTRL1, &pmsc->pmsc_ctrl1, sizeof(pmsc->pmsc_ctrl1), NULL))
        goto err;

    if (m_dw1000_ctx.lde_run_enable) {
        // Turn off LDERUNE
        pmsc->pmsc_ctrl1.lderune = 0;
        if (dw1000_short_indexed_write(spi_cfg, DW1000_PMSC, DW1000_PMSC_CTRL1, &pmsc->pmsc_ctrl1, sizeof(pmsc->pmsc_ctrl1), NULL))
            goto err;

        union DW1000_SUB_REG_PMSC_CTRL0 *pmsc_ctrl0 = &m_dw1000_ctx.pmsc_ctrl0;
        pmsc_ctrl0->word_l = 0x0301;
        if (dw1000_short_indexed_write(spi_cfg, DW1000_PMSC, DW1000_PMSC_CTRL0, &pmsc_ctrl0->word_l, sizeof(pmsc_ctrl0->word_l), NULL))
            goto err;

        union DW1000_REG_OTP_IF *otp_if = &m_dw1000_ctx.otp_if;
        otp_if->otp_ctrl.ldeload = 1;
        if (dw1000_short_indexed_write(spi_cfg, DW1000_OTP_IF, DW1000_OTP_CTRL, &otp_if->otp_ctrl, sizeof(otp_if->otp_ctrl), NULL))
            goto err;

        sleep_us(150);
        pmsc_ctrl0->word_l = 0x0200;
        if (dw1000_short_indexed_write(spi_cfg, DW1000_PMSC, DW1000_PMSC_CTRL0, &pmsc_ctrl0->word_l, sizeof(pmsc_ctrl0->word_l), NULL))
            goto err;

        pmsc->pmsc_ctrl1.lderune = 1;
        if (m_dw1000_ctx.sleep_enable) {
            union DW1000_REG_AON *aon = &m_dw1000_ctx.aon;
            aon->aon_wcfg.onw_llde = 1;
            if (dw1000_short_indexed_write(spi_cfg, DW1000_AON, DW1000_AON_WCFG, &aon->aon_wcfg, sizeof(aon->aon_wcfg), NULL))
                goto err;
        }
    } else {
        pmsc->pmsc_ctrl1.lderune = 0;
    }

    // Turn on LDERUNE
    if (dw1000_short_indexed_write(spi_cfg, DW1000_PMSC, DW1000_PMSC_CTRL1, &pmsc->pmsc_ctrl1, sizeof(pmsc->pmsc_ctrl1), NULL))
        goto err;

    // TODO: LDOTUNE
    // TODO: External Synchronisation

    /* *************************************************************************
     *                          Channel Configuration
     * ************************************************************************/

    union DW1000_REG_CHAN_CTRL *chan_ctrl = &m_dw1000_ctx.chan_ctrl;
    chan_ctrl->tx_chan  = DW1000_CHAN;
    chan_ctrl->rx_chan  = DW1000_CHAN;
    chan_ctrl->rxprf    = DW1000_PRF;
    chan_ctrl->tx_pcode = DW1000_PCODE;
    chan_ctrl->rx_pcode = DW1000_PCODE;
    hard_assert(chan_ctrl->tx_chan == chan_ctrl->rx_chan);
    hard_assert((chan_ctrl->rxprf == DW1000_PRF_16MHZ) || (chan_ctrl->rxprf == DW1000_PRF_64MHZ));
    hard_assert(chan_ctrl->tx_pcode == chan_ctrl->rx_pcode);
    if (dw1000_non_indexed_write(spi_cfg, DW1000_CHAN_CTRL, chan_ctrl, sizeof(*chan_ctrl), "chan_ctrl: "))
        goto err;

    /**
     * The RF PLL and Clock PLL are configured for channel 5 operation by
     * default, please refer to Register file: 0x2B – Frequency synthesiser
     * control block for channel configuration settings for each channel.
     */
    union DW1000_REG_FS_CTRL *fs_ctrl = &m_dw1000_ctx.fs_ctrl;
    switch (chan_ctrl->rx_chan) {
    case 1:
        fs_ctrl->fs_pllcfg.value[0] = 0x07;
        fs_ctrl->fs_pllcfg.value[1] = 0x04;
        fs_ctrl->fs_pllcfg.value[2] = 0x00;
        fs_ctrl->fs_pllcfg.value[3] = 0x09;
        fs_ctrl->fs_plltune.value   = 0x1E;
        break;
    case 2:
    case 4:
        fs_ctrl->fs_pllcfg.value[0] = 0x08;
        fs_ctrl->fs_pllcfg.value[1] = 0x05;
        fs_ctrl->fs_pllcfg.value[2] = 0x40;
        fs_ctrl->fs_pllcfg.value[3] = 0x08;
        fs_ctrl->fs_plltune.value   = 0x26;
        break;
    case 3:
        fs_ctrl->fs_pllcfg.value[0] = 0x09;
        fs_ctrl->fs_pllcfg.value[1] = 0x10;
        fs_ctrl->fs_pllcfg.value[2] = 0x40;
        fs_ctrl->fs_pllcfg.value[3] = 0x08;
        fs_ctrl->fs_plltune.value   = 0x56;
        break;
    case 5:
    case 7:
        fs_ctrl->fs_pllcfg.value[0] = 0x1D;
        fs_ctrl->fs_pllcfg.value[1] = 0x04;
        fs_ctrl->fs_pllcfg.value[2] = 0x00;
        fs_ctrl->fs_pllcfg.value[3] = 0x08;
        fs_ctrl->fs_plltune.value   = 0xBE;
        break;
    default:
        hard_assert(0);
    };
    // TODO: IC Calibration – Crystal Oscillator Trim
    if (dw1000_short_indexed_write(spi_cfg, DW1000_FS_CTRL, DW1000_FS_PLLCFG, &fs_ctrl->fs_pllcfg, sizeof(fs_ctrl->fs_pllcfg), "fs_pllcfg: "))
        goto err;

    /**
     * FS_PLLTUNE is set to 0x46 by default, which is not the optimal value for channel 5.
     */
    if (dw1000_short_indexed_write(spi_cfg, DW1000_FS_CTRL, DW1000_FS_PLLTUNE, &fs_ctrl->fs_plltune, sizeof(fs_ctrl->fs_plltune), "fs_plltune: "))
        goto err;

    /* *************************************************************************
     *                      Transmitter Configuration
     * ************************************************************************/

    /**
     * The transmit data rate is set to 6.8 Mbps in the TX_FCTRL register, see
     * TXBR field in Register file: 0x08 – Transmit Frame Control . The receive
     * data rate is never set unless 110 kbps reception is required. Note that
     * this must be configured in register SYS_CFG, field RXM110K, see Register
     * file: 0x04 – System Configuration.
     */
    union DW1000_REG_TX_FCTRL *tx_fctrl = &m_dw1000_ctx.tx_fctrl;
    tx_fctrl->ofs_00.tflen    = 12;  // 8 + 4 bytes
    tx_fctrl->ofs_00.txbr     = DW1000_BR;
    tx_fctrl->ofs_00.tr       = 1;
    tx_fctrl->ofs_00.txprf    = DW1000_PRF;
    tx_fctrl->ofs_00.txpsr    = DW1000_PSR & 0x3;
    tx_fctrl->ofs_00.pe       = DW1000_PSR >> 2;
    uint8_t psr = (tx_fctrl->ofs_00.pe << 2) | tx_fctrl->ofs_00.txpsr;
    const char *_txbr[] = {
        "110 kbps",
        "850 kbps",
        "6.8 Mbps",
        "Reserved"
    };
    printf("Bit Rate                         : %s (%d)\n", _txbr[tx_fctrl->ofs_00.txbr], tx_fctrl->ofs_00.txbr);
    const char *_txprf[] = {
        "4 MHz",
        "16 MHz",
        "64 MHz",
        "Reserved"
    };
    printf("Nominal PRF                      : %s (%d)\n", _txprf[tx_fctrl->ofs_00.txprf], tx_fctrl->ofs_00.txprf);
    const uint16_t _txpsr[] = {
        [0x1] = 64,
        [0x2] = 1024,
        [0x3] = 4096,
        [0x5] = 128,
        [0x9] = 256,
        [0xd] = 512,
        [0x6] = 1536,
        [0xa] = 2048,
    };
    printf("Preamble Length                  : %d (%x,%x)\n", _txpsr[psr] , tx_fctrl->ofs_00.txpsr, tx_fctrl->ofs_00.pe);
    hard_assert(tx_fctrl->ofs_00.tflen <= DW1000_TX_BUFFER_SIZE);
    hard_assert(!((sys_cfg->rxm110k == true) ^ (tx_fctrl->ofs_00.txbr == DW1000_BR_110KBPS)));
    hard_assert((tx_fctrl->ofs_00.txprf == DW1000_PRF_16MHZ) || (tx_fctrl->ofs_00.txprf == DW1000_PRF_64MHZ));
    hard_assert((tx_fctrl->ofs_00.txprf == chan_ctrl->rxprf));
    if (dw1000_non_indexed_write(spi_cfg, DW1000_TX_FCTRL, tx_fctrl, sizeof(*tx_fctrl), "tx_fctrl: "))
        goto err;

    m_dw1000_ctx.is_txprf_16mhz = ((tx_fctrl->ofs_00.txprf == DW1000_PRF_16MHZ) ? true : false);
    bool is_txprf_16mhz = m_dw1000_ctx.is_txprf_16mhz;

    /**
     * The TX_POWER setting is 0x0E080222 by default. This value should be set
     * to 0x0E082848 before proceeding to use the default configuration.
     */
    union DW1000_REG_TX_POWER *tx_power = &m_dw1000_ctx.tx_power;
    if (sys_cfg->dis_stxp == false) {
        // Transmit Power Control, for Smart Transmit Power Control
        switch (chan_ctrl->tx_chan) {
        case 1:
        case 2:
            tx_power->value = (is_txprf_16mhz ? 0x15355575 : 0x07274767);
            break;
        case 3:
            tx_power->value = (is_txprf_16mhz ? 0x0F2F4F6F : 0x2B4B6B8B);
            break;
        case 4:
            tx_power->value = (is_txprf_16mhz ? 0x1F1F3F5F : 0x3A5A7A9A);
            break;
        case 5:
            tx_power->value = (is_txprf_16mhz ? 0x0E082848 : 0x25466788);
            break;
        case 7:
            tx_power->value = (is_txprf_16mhz ? 0x32527292 : 0x5171B1D1);
            break;
        default:
            hard_assert(0);
        }
    } else {
        // Transmit Power Control for Manual Transmit Power Control
        switch (chan_ctrl->tx_chan) {
        case 1:
        case 2:
            tx_power->value = (is_txprf_16mhz ? 0x75757575 : 0x67676767);
            break;
        case 3:
            tx_power->value = (is_txprf_16mhz ? 0x6F6F6F6F : 0x8B8B8B8B);
            break;
        case 4:
            tx_power->value = (is_txprf_16mhz ? 0x5F5F5F5F : 0x9A9A9A9A);
            break;
        case 5:
            tx_power->value = (is_txprf_16mhz ? 0x48484848 : 0x85858585);
            break;
        case 7:
            tx_power->value = (is_txprf_16mhz ? 0x92929292 : 0xD1D1D1D1);
            break;
        default:
            hard_assert(0);
        }
    }
    if (dw1000_non_indexed_write(spi_cfg, DW1000_TX_POWER, tx_power, sizeof(*tx_power), "tx_power: "))
        goto err;

    /* *************************************************************************
     *                         Receiver Configuration
     * ************************************************************************/

    /**
     * Digital receiver tuning registers; DRX_TUNE0b, DRX_TUNE1a, DRX_TUNE1b and
     * DRX_TUNE2 are configured by default for 16 MHz PRF, 6.8 Mbps data rate
     * and a preamble symbol repetition of length 128.
     */
    union DW1000_REG_DRX_CONF *drx_conf = &m_dw1000_ctx.drx_conf;
    m_dw1000_ctx.is_standard_sfd = ((chan_ctrl->dwsfd || chan_ctrl->tnssfd || chan_ctrl->rnssfd) ? false : true);
    printf("Start of Frame Delimiter         : %s (%d,%d,%d)\n",
        (m_dw1000_ctx.is_standard_sfd   ? "Standard SFD" : "Standard SFD"),
        chan_ctrl->dwsfd, chan_ctrl->tnssfd, chan_ctrl->rnssfd);

    bool is_standard_sfd = m_dw1000_ctx.is_standard_sfd;
    switch (tx_fctrl->ofs_00.txbr) {
    case DW1000_BR_110KBPS:
        drx_conf->drx_tune0b.value = (is_standard_sfd ? 0x000A : 0x0016);
        break;
    case DW1000_BR_850KBPS:
        drx_conf->drx_tune0b.value = (is_standard_sfd ? 0x0001 : 0x0006);
        break;
    case DW1000_BR_6800KBPS:
        drx_conf->drx_tune0b.value = (is_standard_sfd ? 0x0001 : 0x0002);
        break;
    default:
        hard_assert(0);
    }
    if (dw1000_short_indexed_write(spi_cfg, DW1000_DRX_CONF, DW1000_DRX_TUNE0b, &drx_conf->drx_tune0b, sizeof(drx_conf->drx_tune0b), "drx_tune0b: "))
        goto err;

    drx_conf->drx_tune1a.value = (chan_ctrl->rxprf == DW1000_PRF_16MHZ ? 0x0087 : 0x008D);
    if (dw1000_short_indexed_write(spi_cfg, DW1000_DRX_CONF, DW1000_DRX_TUNE1a, &drx_conf->drx_tune1a, sizeof(drx_conf->drx_tune1a), "drx_tune1a: "))
        goto err;

    switch (psr) {
    // Preamble length = 64 symbols, for 6.8 Mbps operation
    case DW1000_PSR_64:
        drx_conf->drx_tune1b.value = 0x0010;
        break;
    // Preamble lengths 128 to 1024 symbols, for 850 kbps and 6.8 Mbps operation
    case DW1000_PSR_128:
    case DW1000_PSR_256:
    case DW1000_PSR_512:
    case DW1000_PSR_1024:
        drx_conf->drx_tune1b.value = 0x0020;
        break;
    // Preamble lengths > 1024 symbols, for 110 kbps operation
    case DW1000_PSR_1536:
    case DW1000_PSR_2048:
    case DW1000_PSR_4096:
        drx_conf->drx_tune1b.value = 0x0064;
        break;
    default:
        hard_assert(0);
    }
    if (dw1000_short_indexed_write(spi_cfg, DW1000_DRX_CONF, DW1000_DRX_TUNE1b, &drx_conf->drx_tune1b, sizeof(drx_conf->drx_tune1b), "drx_tune1b: "))
        goto err;

    switch (psr) {
    // Recommended PAC size: 8
    case DW1000_PSR_64:
    case DW1000_PSR_128:
        drx_conf->drx_tune2.value = (chan_ctrl->rxprf == DW1000_PRF_16MHZ ? 0x311A002D : 0x313B006B);
        break;
    // Recommended PAC size: 16
    case DW1000_PSR_256:
    case DW1000_PSR_512:
        drx_conf->drx_tune2.value = (chan_ctrl->rxprf == DW1000_PRF_16MHZ ? 0x331A0052 : 0x333B00BE);
        break;
    // Recommended PAC size: 32
    case DW1000_PSR_1024:
        drx_conf->drx_tune2.value = (chan_ctrl->rxprf == DW1000_PRF_16MHZ ? 0x351A009A : 0x353B015E);
        break;
    // Recommended PAC size: 64
    case DW1000_PSR_1536:
    case DW1000_PSR_2048:
    case DW1000_PSR_4096:
        drx_conf->drx_tune2.value = (chan_ctrl->rxprf == DW1000_PRF_16MHZ ? 0x371A011D : 0x373B0296);
        break;
    default:
        hard_assert(0);
    }
    if (dw1000_short_indexed_write(spi_cfg, DW1000_DRX_CONF, DW1000_DRX_TUNE2, &drx_conf->drx_tune2, sizeof(drx_conf->drx_tune2), "drx_tune2: "))
        goto err;

    /**
     * whilst SFD detection timeout (see Sub-Register 0x27:20 – DRX_SFDTOC) is on.
     */
    drx_conf->drx_stdtoc.value = 4096 + 64 + 1;
    if (dw1000_short_indexed_write(spi_cfg, DW1000_DRX_CONF, DW1000_DRX_SFDTOC, &drx_conf->drx_stdtoc, sizeof(drx_conf->drx_stdtoc), "drx_stdtoc: "))
        goto err;

    /**
     * preamble detection timeout (see Sub-Register 0x27:24 – DRX_PRETOC) are off,
     */
    drx_conf->drx_pretoc.value = 0;
    if (dw1000_short_indexed_write(spi_cfg, DW1000_DRX_CONF, DW1000_DRX_PRETOC, &drx_conf->drx_pretoc, sizeof(drx_conf->drx_pretoc), "drx_pretoc: "))
        goto err;

    /**
     * Register file: 0x27 – Digital receiver configuration, sub-register 0x26
     * is a 16-bit tuning register. The value here needs to change depending on
     * the preamble length expected by the receiver.
     */
    drx_conf->drx_tune4h.value = (psr == DW1000_PSR_64 ? 0x0010 : 0x0028);
    if (dw1000_short_indexed_write(spi_cfg, DW1000_DRX_CONF, DW1000_DRX_TUNE4H, &drx_conf->drx_tune4h, sizeof(drx_conf->drx_tune4h), "drx_tune4h: "))
        goto err;

    /**
     * Receiver RF channel configurations are set for channel 5 by default
     */
    union DW1000_REG_RF_CONF *rf_conf = &m_dw1000_ctx.rf_conf;
    switch (chan_ctrl->rx_chan) {
    case 1:
    case 2:
    case 3:
    case 5:
        rf_conf->rf_rxctrlh.value = 0xD8;
        break;
    case 4:
    case 7:
        rf_conf->rf_rxctrlh.value = 0xBC;
        break;
    }
    if (dw1000_short_indexed_write(spi_cfg, DW1000_RF_CONF, DW1000_RF_RXCTRLH, &rf_conf->rf_rxctrlh, sizeof(rf_conf->rf_rxctrlh), "rf_rxctrlh: "))
        goto err;

    /**
     * RF_TXCTRL is not set to the optimum values by default.
     */
    switch (chan_ctrl->tx_chan) {
    case 1:
        // 0x00 00 5C 40
        rf_conf->rf_txctrl.value[0] = 0x40;
        rf_conf->rf_txctrl.value[1] = 0x5C;
        rf_conf->rf_txctrl.value[2] = 0x00;
        break;
    case 2:
        // 0x00 04 5C A0
        rf_conf->rf_txctrl.value[0] = 0xA0;
        rf_conf->rf_txctrl.value[1] = 0x5C;
        rf_conf->rf_txctrl.value[2] = 0x04;
        break;
    case 3:
        // 0x00 08 6C C0
        rf_conf->rf_txctrl.value[0] = 0xC0;
        rf_conf->rf_txctrl.value[1] = 0x6C;
        rf_conf->rf_txctrl.value[2] = 0x08;
        break;
    case 4:
        // 0x00 04 5C 80
        rf_conf->rf_txctrl.value[0] = 0x80;
        rf_conf->rf_txctrl.value[1] = 0x5C;
        rf_conf->rf_txctrl.value[2] = 0x04;
        break;
    case 5:
        // 0x00 1E 3F E3
        rf_conf->rf_txctrl.value[0] = 0xE3;
        rf_conf->rf_txctrl.value[1] = 0x3F;
        rf_conf->rf_txctrl.value[2] = 0x1E;
        break;
    case 7:
        // 0x00 1E 7D E0
        rf_conf->rf_txctrl.value[0] = 0xE0;
        rf_conf->rf_txctrl.value[1] = 0x7D;
        rf_conf->rf_txctrl.value[2] = 0x1E;
        break;
    }
    if (dw1000_short_indexed_write(spi_cfg, DW1000_RF_CONF, DW1000_RF_TXCTRL, &rf_conf->rf_txctrl, sizeof(rf_conf->rf_txctrl), "rf_txctrl: "))
        goto err;

    /* *************************************************************************
     *            Default Configurations that should be modified
     * ************************************************************************/

    /**
     * AGC_TUNE1 is set to 0x889B by default which is not the optimal value for
     * the default PRF of 16 MHz.
     */
    union DW1000_REG_AGC_CTRL *agc_ctrl = &m_dw1000_ctx.agc_ctrl;
    agc_ctrl->agc_tune1.value = (is_txprf_16mhz ? 0x8870 : 0x889B);
    if (dw1000_short_indexed_write(spi_cfg, DW1000_AGC_CTRL, DW1000_AGC_TUNE1, &agc_ctrl->agc_tune1, sizeof(agc_ctrl->agc_tune1), "agc_tune1: "))
        goto err;

    /**
     * The default value of this register needs to be reconfigured for optimum
     * operation of the AGC.
     */
    agc_ctrl->agc_tune2.value = 0x2502a907;
    if (dw1000_short_indexed_write(spi_cfg, DW1000_AGC_CTRL, DW1000_AGC_TUNE2, &agc_ctrl->agc_tune2, sizeof(agc_ctrl->agc_tune2), "agc_tune2: "))
        goto err;

    /**
     * NTM is set to 0xC by default and may be set to 0xD for better performance.
     */
    union DW1000_SUB_REG_LDE_CFG1 *lde_cfg1 = &m_dw1000_ctx.lde_cfg1;
    if (dw1000_long_indexed_read(spi_cfg, DW1000_LDE_CTRL, DW1000_LDE_CFG1, lde_cfg1, sizeof(*lde_cfg1), NULL))
        goto err;
    lde_cfg1->ntm = 0xD;
    if (dw1000_long_indexed_write(spi_cfg, DW1000_LDE_CTRL, DW1000_LDE_CFG1, lde_cfg1, sizeof(*lde_cfg1), "lde_cfg1: "))
        goto err;

    /**
     * LDE_CFG2 is set to 0x0000 by default and should be set to 0x1607 for 16
     * MHz PRF before proceeding to use the default configuration.
     */
    union DW1000_SUB_REG_LDE_CFG2 *lde_cfg2 = &m_dw1000_ctx.lde_cfg2;
    lde_cfg2->value = (is_txprf_16mhz ? 0x1607 : 0x0607);
    if (dw1000_long_indexed_write(spi_cfg, DW1000_LDE_CTRL, DW1000_LDE_CFG2, lde_cfg2, sizeof(*lde_cfg2), "lde_cfg2: "))
        goto err;

    union DW1000_SUB_REG_LDE_REPC *lde_repc = &m_dw1000_ctx.lde_repc;
    uint16_t _lde_repc[24] = {
        [0]  = 0x5998,
        [1]  = 0x5998,
        [2]  = 0x51EA,
        [3]  = 0x428E,
        [4]  = 0x451E,
        [5]  = 0x2E14,
        [6]  = 0x8000,
        [7]  = 0x51EA,
        [8]  = 0x28F4,
        [9]  = 0x3332,
        [10] = 0x3AE0,
        [11] = 0x3D70,
        [12] = 0x3AE0,
        [13] = 0x35C2,
        [14] = 0x2B84,
        [15] = 0x35C2,
        [16] = 0x3332,
        [17] = 0x35C2,
        [18] = 0x35C2,
        [19] = 0x47AE,
        [20] = 0x3AE0,
        [21] = 0x3850,
        [22] = 0x30A2,
        [23] = 0x3850,
    };
    uint16_t temp = _lde_repc[chan_ctrl->rx_pcode - 1];
    lde_repc->value = (tx_fctrl->ofs_00.txbr == DW1000_BR_110KBPS ? temp >> 3 : temp);
    if (dw1000_long_indexed_write(spi_cfg, DW1000_LDE_CTRL, DW1000_LDE_REPC, lde_repc, sizeof(*lde_repc), "lde_repc: "))
        goto err;

    /**
     * TC_PGDELAY is set to 0xC5 by default, which is the incorrect value for channel 5.
     */
    union DW1000_SUB_REG_TC_PGDELAY *tc_pgdelay = &m_dw1000_ctx.tc_pgdelay;
    switch (chan_ctrl->tx_chan) {
    case 1:
        tc_pgdelay->value = 0xC9;
        break;
    case 2:
        tc_pgdelay->value = 0xC2;
        break;
    case 3:
        tc_pgdelay->value = 0xC5;
        break;
    case 4:
        tc_pgdelay->value = 0x95;
        break;
    case 5:
        tc_pgdelay->value = 0xB5;
        break;
    case 7:
        tc_pgdelay->value = 0x93;
        break;
    default:
        hard_assert(0);
    }
    if (dw1000_short_indexed_write(spi_cfg, DW1000_TX_CAL, DW1000_TC_PGDELAY, tc_pgdelay, sizeof(*tc_pgdelay), "tc_pgdelay: "))
        goto err;

    // Clear the interrupt status
    if (dw1000_clear_sys_status(spi_cfg))
        goto err;

    // Set the interrupt mask
    union DW1000_REG_SYS_MASK sys_mask = {.value = DW1000_SYS_STS_MASK};
    if (dw1000_non_indexed_write(spi_cfg, DW1000_SYS_MASK, &sys_mask, sizeof(sys_mask), NULL))
        goto err;
    m_dw1000_ctx.sys_mask = sys_mask;

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

/**
 * @brief Estimating the signal power in the first path.
 */
float dw1000_cal_first_path_power_level()
{
    const struct spi_config *spi_cfg = &m_dw1000_ctx.spi_cfg;
    union DW1000_REG_RX_TIME rx_time = {0};
    if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_TIME, &rx_time, sizeof(rx_time), NULL))
        goto err;

    union DW1000_REG_RX_FQUAL rx_fqual = {0};
    if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_FQUAL, &rx_fqual, sizeof(rx_fqual), NULL))
        goto err;

    union DW1000_REG_RX_FINFO rx_finfo = {0};
    if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_FINFO, &rx_finfo, sizeof(rx_finfo), NULL))
        goto err;

    float f1 = (float)(((uint16_t)rx_time.fp_ampl1_h << 8) | (uint16_t)rx_time.fp_ampl1_l);
    float f2 = (float)(rx_fqual.fp_ampl2);
    float f3 = (float)(rx_fqual.fp_ampl3);
    float a  = (float)(m_dw1000_ctx.chan_ctrl.rxprf == DW1000_PRF_16MHZ ? 113.77f : 121.74f);
    float n  = (float)rx_finfo.rxpacc;
    if (n <= 0.0f)
        return NAN;

    float sum_of_squares = (f1 * f1) + (f2 * f2) + (f3 * f3);
    if (sum_of_squares <= 0.0f)
        return -INFINITY;

    return 10.0f * log10f(sum_of_squares) - 20.0f * log10f(n) - a;
err:
    printf("%s failed\n", __func__);
    return NAN;
}

/**
 * @brief Estimating the receive signal power.
 */
float dw1000_cal_rx_power_level()
{
    const struct spi_config *spi_cfg = &m_dw1000_ctx.spi_cfg;
    union DW1000_REG_RX_FQUAL rx_fqual = {0};
    if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_FQUAL, &rx_fqual, sizeof(rx_fqual), NULL))
        goto err;

    union DW1000_REG_RX_FINFO rx_finfo = {0};
    if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_FINFO, &rx_finfo, sizeof(rx_finfo), NULL))
        goto err;

    float c = (float)rx_fqual.cir_pwr;
    if (c <= 0.0f)
        return -INFINITY;
    float a = (float)(m_dw1000_ctx.chan_ctrl.rxprf == DW1000_PRF_16MHZ ? 113.77f : 121.74f);
    float n = (float)rx_finfo.rxpacc;
    if (n <= 0.0f)
        return NAN;

    return 10.0f * log10f(c) + 170.0f * log10f(2.0f) - 20.0f * log10f(n) - a;
err:
    printf("%s failed\n", __func__);
    return NAN;
}

/**
 * @brief Get Host Side Receive Buffer Pointer
 */
int dw1000_get_rx_buf_ptr(const struct spi_config *spi_cfg)
{
    union DW1000_REG_SYS_STATUS sys_sts = {0};
    if (dw1000_non_indexed_read(spi_cfg, DW1000_SYS_STATUS, &sys_sts, sizeof(sys_sts), NULL))
        goto err;

    return (sys_sts.ofs_00.icrbp << 1) | sys_sts.ofs_00.hsrbp;
err:
    printf("%s failed\n", __func__);
    return -1;
}

/**
 * @brief Toggle Host Side Receive Buffer Pointer
 */
int dw1000_set_rx_buf_ptr(const struct spi_config *spi_cfg)
{
    union DW1000_REG_SYS_CTRL sys_ctrl = {
        sys_ctrl.hrbpt = 1
    };
    if (dw1000_non_indexed_write(spi_cfg, DW1000_SYS_CTRL, &sys_ctrl, sizeof(sys_ctrl), NULL))
        goto err;

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

int dw1000_rx_start(const struct spi_config *spi_cfg)
{
    union DW1000_REG_SYS_CTRL sys_ctrl = {.rxenab = 1};
    if (dw1000_non_indexed_write(spi_cfg, DW1000_SYS_CTRL, &sys_ctrl, sizeof(sys_ctrl), NULL)) {
        printf("%s failed\n", __func__);
        return -1;
    }

    return 0;
}

/**
 * @brief In order to transmit, the host controller must write data for transmission
 * to Register file: 0x09 – Transmit Data Buffer.
 */
int dw1000_prepare_tx_buffer(const struct spi_config *spi_cfg, void *buf, size_t len)
{
    if (len > DW1000_TX_BUFFER_SIZE)
        goto err;

    if (dw1000_non_indexed_write(spi_cfg, DW1000_TX_BUFFER, buf, len, NULL))
        goto err;

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

/**
 * @brief Transmit a data frame through the DW1000 transceiver.
 *
 * This function initiates a data transmission using the DW1000 device.
 * It performs parameter validation, loads the transmit buffer via SPI,
 * and triggers transmission by setting the TXSTRT bit in the SYS_CTRL register.
 *
 * Transmission flow:
 * 1. Validate input parameters and frame length against TX_FCTRL configuration.
 * 2. Write the payload buffer into the DW1000 TX buffer memory.
 * 3. Set `SYS_CTRL.TXSTRT = 1` to start transmission.
 *
 * @param[in] buf        Pointer to the payload buffer to transmit.
 * @param[in] len        Length of the payload in bytes.
 *
 * @retval 0  Transmission started successfully.
 * @retval -1 Parameter error, SPI access failure, or invalid frame length.
 *
 * @note The function only triggers transmission; it does not wait for
 *       completion or check for TX confirmation interrupts.
 *       The user should monitor the SYS_STATUS.TXFRS bit to determine
 *       when transmission is complete.
 */
int dw1000_transmit_message(void *buf, size_t len, bool wait4resp)
{
    if ((buf == NULL) || ((len + 2) > m_dw1000_ctx.tx_fctrl.ofs_00.tflen)) {
        printf("invalid transmission lengh:%zu,%d\n", len, m_dw1000_ctx.tx_fctrl.ofs_00.tflen);
        goto err;
    }

    const struct spi_config *spi_cfg = &m_dw1000_ctx.spi_cfg;

    // print_buf(m_dw1000_ctx.tx_fctrl, sizeof(m_dw1000_ctx.tx_fctrl), "DW1000_TX_FCTRL 1\n");
    // uint8_t _tx_fctrl[5];
    // if (dw1000_non_indexed_read(spi_cfg, DW1000_TX_FCTRL, _tx_fctrl, sizeof(_tx_fctrl), NULL))
    //     goto err;
    // print_buf(_tx_fctrl, sizeof(*tx_fctrl), "DW1000_TX_FCTRL 2\n");

    if (dw1000_clear_sys_status_ofs_00_by_mask(spi_cfg, DW1000_SYS_STS_TXFRB |
        DW1000_SYS_STS_TXPRS | DW1000_SYS_STS_TXPHS | DW1000_SYS_STS_TXFRS))
        goto err;

    if (dw1000_prepare_tx_buffer(spi_cfg, buf, len))
        goto err;

    union DW1000_REG_SYS_CTRL sys_ctrl = {.txstrt = 1, .wait4resp = !!wait4resp};
    if (dw1000_non_indexed_write(spi_cfg, DW1000_SYS_CTRL, &sys_ctrl, sizeof(sys_ctrl), NULL))
        goto err;

    pico_set_led(led_out);
    led_out = !led_out;

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

void dw1000_isr()
{
    const struct spi_config *spi_cfg = &m_dw1000_ctx.spi_cfg;
    union DW1000_REG_SYS_STATUS *sys_status = &m_dw1000_ctx.sys_status;
    if (dw1000_non_indexed_read(spi_cfg, DW1000_SYS_STATUS, sys_status, sizeof(*sys_status), NULL))
        goto err;

    if (m_dw1000_ctx.ads_twr_state != DW1000_ADS_TWR_STATE_LISTEN)
    {
        m_dw1000_ctx.listen_to = false;
        print_buf(sys_status, sizeof(*sys_status), "\nisr: ");
    }
    else
    {
        m_dw1000_ctx.listen_to = true;
    }

    if (sys_status->ofs_00.value & (DW1000_SYS_STS_RXFCG | DW1000_SYS_STS_RXDFR))
    {
        pico_set_led(led_out);
        led_out = !led_out;

        // printf("rxf:%d-%d-(%d,%d)-%d-%d-(%d,%d)\n",
        //     sys_status->ofs_00.rxprd, sys_status->ofs_00.rxsfdd, sys_status->ofs_00.rxphd, sys_status->ofs_00.rxphe,
        //     sys_status->ofs_00.ldedone, sys_status->ofs_00.rxdfr,sys_status->ofs_00.rxfcg, sys_status->ofs_00.rxfce);

        // union DW1000_REG_RX_FINFO rx_finfo;
        // if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_FINFO, &rx_finfo, sizeof(rx_finfo), NULL))
        //     goto err;
        // printf("rxflen:%d\n", rx_finfo.rxflen);

        // if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_BUFFER, m_rx_buf, rx_finfo.rxflen, NULL))
        //     goto err;
        // print_buf(m_rx_buf, rx_finfo.rxflen, "data:\n");

        // if (dw1000_rx_start(spi_cfg))
        //     goto err;
    }
    else if (sys_status->ofs_00.value & DW1000_SYS_STS_RXRFTO)
    {
        if (m_dw1000_ctx.ads_twr_state != DW1000_ADS_TWR_STATE_LISTEN)
        {
            printf("rxrfto\n");
        }
    }
    else if (sys_status->ofs_00.value & (DW1000_SYS_STS_RXFSL | DW1000_SYS_STS_RXFCE | DW1000_SYS_STS_RXPHE))
    {
        print_buf(sys_status, sizeof(*sys_status), "\nre00:\n");
        printf("rxf:%d-%d-(%d,%d)-%d-%d-(%d,%d)\n",
            sys_status->ofs_00.rxprd, sys_status->ofs_00.rxsfdd, sys_status->ofs_00.rxphd, sys_status->ofs_00.rxphe,
            sys_status->ofs_00.ldedone, sys_status->ofs_00.rxdfr,sys_status->ofs_00.rxfcg, sys_status->ofs_00.rxfce);
    #if (!CONFIG_DW1000_AUTO_RX)
        if (dw1000_rx_start(spi_cfg))
            goto err;
    #endif
    }
    else if (sys_status->ofs_00.value & 0x3FFF9000)
    {
        print_buf(sys_status, sizeof(*sys_status), "\nmics:\n");
        printf("rxf:%d-%d-(%d,%d)-%d-%d-(%d,%d)\n",
            sys_status->ofs_00.rxprd, sys_status->ofs_00.rxsfdd, sys_status->ofs_00.rxphd, sys_status->ofs_00.rxphe,
            sys_status->ofs_00.ldedone, sys_status->ofs_00.rxdfr,sys_status->ofs_00.rxfcg, sys_status->ofs_00.rxfce);
    #if (!CONFIG_DW1000_AUTO_RX)
        if (dw1000_rx_start(spi_cfg))
            goto err;
    #endif
    }
    else if (sys_status->ofs_04.value & (DW1000_SYS_STS_TXPUTE | DW1000_SYS_STS_RXRSCS))
    {
        print_buf(sys_status, sizeof(*sys_status), "\nre04:\n");
        printf("rxf:%d-%d-(%d,%d)-%d-%d-(%d,%d)\n",
            sys_status->ofs_00.rxprd, sys_status->ofs_00.rxsfdd, sys_status->ofs_00.rxphd, sys_status->ofs_00.rxphe,
            sys_status->ofs_00.ldedone, sys_status->ofs_00.rxdfr,sys_status->ofs_00.rxfcg, sys_status->ofs_00.rxfce);
    #if (!CONFIG_DW1000_AUTO_RX)
        if (dw1000_rx_start(spi_cfg))
            goto err;
    #endif
    }

    // pico_set_led(led_out);
    // led_out = !led_out;

    if (dw1000_clear_sys_status(spi_cfg))
        goto err;

    return;
err:
    printf("%s failed.\n", __func__);
    return;
}

int driver_dw1000_gpio_init()
{
    printf("%s\n", __func__);

    m_dw1000_ctx.gpio_rst_cfg.pin = RSTn_PIN;
    gpio_init(m_dw1000_ctx.gpio_rst_cfg.pin);
    gpio_set_dir(m_dw1000_ctx.gpio_rst_cfg.pin, GPIO_OUT);

    return 0;
}

int driver_dw1000_gpio_irq_init()
{
    printf("%s\n", __func__);

    struct gpio_config *gpio_irq_cfg = &m_dw1000_ctx.gpio_irq_cfg;
    gpio_irq_cfg->pin        = IRQ_PIN;
    gpio_irq_cfg->enabled    = true;
    // gpio_irq_cfg->event_mask = GPIO_IRQ_EDGE_RISE;
    gpio_irq_cfg->event_mask = GPIO_IRQ_LEVEL_HIGH;
    gpio_irq_cfg->callback   = dw1000_isr;

    if (gpio_irq_init(gpio_irq_cfg))
        goto err;

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

int driver_dw1000_spi_init()
{
    printf("%s\n", __func__);

    struct spi_config *spi_cfg = &m_dw1000_ctx.spi_cfg;
    spi_cfg->spi        = SPI_INST;
    spi_cfg->spi_speed  = SPI_SPEED;
    spi_cfg->pin.sck    = SPI0_SCK_PIN;
    spi_cfg->pin.tx     = SPI0_TX_PIN;
    spi_cfg->pin.rx     = SPI0_RX_PIN;
    spi_cfg->pin.csn    = SPI0_CSN_PIN;
    spi_cfg->slave_mode = false;

    spi_init(spi_cfg->spi, spi_cfg->spi_speed);
    spi_set_slave(spi_cfg->spi, !!spi_cfg->slave_mode);
    gpio_set_function(spi_cfg->pin.sck, GPIO_FUNC_SPI);
    gpio_set_function(spi_cfg->pin.tx, GPIO_FUNC_SPI);
    gpio_set_function(spi_cfg->pin.rx, GPIO_FUNC_SPI);
    // gpio_set_function(spi_cfg->pin.csn, GPIO_FUNC_SPI);

    // Make the SPI pins available to picotool
    // bi_decl(bi_4pins_with_func(SPI0_RX_PIN, SPI0_TX_PIN, SPI0_SCK_PIN, SPI0_CSN_PIN, GPIO_FUNC_SPI));
    // bi_decl(bi_3pins_with_func(SPI0_RX_PIN, SPI0_TX_PIN, SPI0_SCK_PIN, GPIO_FUNC_SPI));

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(spi_cfg->pin.csn);
    gpio_put(spi_cfg->pin.csn, 1);
    gpio_set_dir(spi_cfg->pin.csn, GPIO_OUT);

    // Make the CS pin available to picotool
    // bi_decl(bi_1pin_with_name(SPI0_CSN_PIN, "SPI CS"));

    return 0;
err:
    printf("%s failed\n", __func__);
    return -1;
}

void dw1000_ctx_init()
{
    memset(&m_dw1000_ctx, 0, sizeof(m_dw1000_ctx));
    m_dw1000_ctx.lde_run_enable = true;
#if (CONFIG_DW1000_TAG)
    m_dw1000_ctx.my_addr = 0xAA;
#endif
#if (CONFIG_DW1000_ANCHOR)
    m_dw1000_ctx.my_addr = 0xCC;
#endif
}

void dw1000_unit_test()
{
    printf("%s\n", __func__);

    if (driver_dw1000_gpio_init())
        return;

    if (driver_dw1000_gpio_irq_init())
        return;

    if (driver_dw1000_spi_init())
        return;

    if (dw1000_init())
        return;

    const struct spi_config *spi_cfg = &m_dw1000_ctx.spi_cfg;
    if (dw1000_dump_all_regs(spi_cfg))
        goto err;

#if (CONFIG_DW1000_ANCHOR)
    m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_RX_INIT;
    while (1) {
        volatile union DW1000_REG_SYS_STATUS *sys_status = &m_dw1000_ctx.sys_status;
        switch (m_dw1000_ctx.ads_twr_state) {
        case DW1000_ADS_TWR_STATE_RX_INIT:
        {
            m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_LISTEN;
            if (m_dw1000_ctx.listen_to)
                m_dw1000_ctx.listen_to = 0;
            else
                printf("-> listen\n");

            if (dw1000_rx_start(spi_cfg))
                goto err;
            break;
        }
        // Discovery phase
        case DW1000_ADS_TWR_STATE_LISTEN:
        {
            if (sys_status->ofs_00.rxfcg) {
                sys_status->ofs_00.value = 0;

                union DW1000_REG_RX_FINFO rx_finfo;
                if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_FINFO, &rx_finfo, sizeof(rx_finfo), NULL))
                    goto err;
                printf("rxflen:%d\n", rx_finfo.rxflen);
                union ieee_blink_frame_t *rx_frame = (void *)m_dw1000_ctx.rx_buf;
                if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_BUFFER, rx_frame, rx_finfo.rxflen, NULL))
                    goto err;
                print_buf(rx_frame, rx_finfo.rxflen, "blink frame:\n");

                if (rx_frame->fctrl == IEEE_802_15_4_BLINK_CCP_64)
                {
                    m_dw1000_ctx.tar_addr = rx_frame->long_address;
                    m_dw1000_ctx.seq_num  = rx_frame->seq_num;
                    m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_RANGING_INIT;
                    printf("-> ranging init %d\n", m_dw1000_ctx.seq_num);
                }
                else
                {
                    printf("@@ invalid blink\n");
                    m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_RX_INIT;
                }
            }
            else if (sys_status->ofs_00.rxrfto)
            {
                sys_status->ofs_00.value = 0;
                m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_RX_INIT;
            }
            break;
        }
        // Ranging phase
        case DW1000_ADS_TWR_STATE_RANGING_INIT:
        {
            union ieee_rng_request_frame_t *tx_frame = (void *)m_dw1000_ctx.tx_buf;
            tx_frame->fctrl    = IEEE_802_15_4_FCTRL_RANGE_16;
            tx_frame->seq_num  = ++m_dw1000_ctx.seq_num;
            tx_frame->pan_id   = DW1000_PAN_ID;
            tx_frame->dst_addr = m_dw1000_ctx.tar_addr;
            tx_frame->src_addr = m_dw1000_ctx.my_addr;
            tx_frame->code     = DW1000_TWR_CODE_RNG_INIT;

            printf("-> poll wait %d\n", m_dw1000_ctx.seq_num);
            m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_POLL_WAIT;
            dw1000_transmit_message(tx_frame, sizeof(*tx_frame), true);
            break;
        }
        case DW1000_ADS_TWR_STATE_POLL_WAIT:
        {
            if (sys_status->ofs_00.rxfcg) {
                sys_status->ofs_00.value = 0;

                union DW1000_REG_RX_FINFO rx_finfo;
                if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_FINFO, &rx_finfo, sizeof(rx_finfo), NULL))
                    goto err;
                printf("rxflen:%d\n", rx_finfo.rxflen);

                union ieee_rng_request_frame_t *rx_frame = (void *)m_dw1000_ctx.rx_buf;
                if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_BUFFER, rx_frame, rx_finfo.rxflen, NULL))
                    goto err;
                print_buf(rx_frame, rx_finfo.rxflen, "poll frame:\n");

                if ((rx_frame->fctrl == IEEE_802_15_4_FCTRL_RANGE_16) &&
                    ((m_dw1000_ctx.seq_num + 1) == rx_frame->seq_num) &&
                    (rx_frame->code == DW1000_TWR_CODE_POLL) &&
                    (rx_frame->dst_addr == m_dw1000_ctx.my_addr)) {
                    m_dw1000_ctx.tar_addr = rx_frame->src_addr;
                    m_dw1000_ctx.seq_num  = rx_frame->seq_num;
                    m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_RESPONSE;
                } else {
                    printf("@@ err %d,(%d,%d),%d,%d\n", (rx_frame->fctrl == IEEE_802_15_4_FCTRL_RANGE_16),
                        (m_dw1000_ctx.seq_num + 1), rx_frame->seq_num,
                        (rx_frame->code == DW1000_TWR_CODE_POLL),
                        (rx_frame->dst_addr == m_dw1000_ctx.my_addr));
                    m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_RX_INIT;
                }
            }
            else if (sys_status->ofs_00.rxrfto)
            {
                sys_status->ofs_00.value = 0;
                m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_RX_INIT;
            }
            break;
        }
        case DW1000_ADS_TWR_STATE_RESPONSE:
        {
            union ieee_rng_request_frame_t *tx_frame = (void *)m_dw1000_ctx.tx_buf;
            tx_frame->fctrl    = IEEE_802_15_4_FCTRL_RANGE_16;
            tx_frame->seq_num  = ++m_dw1000_ctx.seq_num;
            tx_frame->pan_id   = DW1000_PAN_ID;
            tx_frame->dst_addr = m_dw1000_ctx.tar_addr;
            tx_frame->src_addr = m_dw1000_ctx.my_addr;
            tx_frame->code     = DW1000_TWR_CODE_RESP;

            printf("-> final wait %d\n", m_dw1000_ctx.seq_num);
            m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_FINAL_WAIT;
            dw1000_transmit_message(tx_frame, sizeof(*tx_frame), true);
            break;
        }
        case DW1000_ADS_TWR_STATE_FINAL_WAIT:
        {
            if (sys_status->ofs_00.rxfcg) {
                sys_status->ofs_00.value = 0;

                union DW1000_REG_RX_FINFO rx_finfo;
                if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_FINFO, &rx_finfo, sizeof(rx_finfo), NULL))
                    goto err;
                printf("rxflen:%d\n", rx_finfo.rxflen);

                union ieee_rng_request_frame_t *rx_frame = (void *)m_dw1000_ctx.rx_buf;
                if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_BUFFER, rx_frame, rx_finfo.rxflen, NULL))
                    goto err;
                print_buf(rx_frame, rx_finfo.rxflen, "final frame:\n");

                if ((rx_frame->fctrl == IEEE_802_15_4_FCTRL_RANGE_16) &&
                    ((m_dw1000_ctx.seq_num + 1) == rx_frame->seq_num) &&
                    (rx_frame->code == DW1000_TWR_CODE_FINAL) &&
                    (rx_frame->dst_addr == m_dw1000_ctx.my_addr)) {
                    m_dw1000_ctx.tar_addr = rx_frame->src_addr;
                    m_dw1000_ctx.seq_num  = rx_frame->seq_num;
                    m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_RX_INIT;
                    printf("@@ final cmpl\n");
                } else {
                    printf("@@ err %d,(%d,%d),%d,%d\n", (rx_frame->fctrl == IEEE_802_15_4_FCTRL_RANGE_16),
                        (m_dw1000_ctx.seq_num + 1), rx_frame->seq_num,
                        (rx_frame->code == DW1000_TWR_CODE_FINAL),
                        (rx_frame->dst_addr == m_dw1000_ctx.my_addr));
                    m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_RX_INIT;
                }
            } else if (sys_status->ofs_00.rxrfto) {
                sys_status->ofs_00.value = 0;
                m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_RX_INIT;
            }
            break;
        }
        default:
            hard_assert(0);
        }
    }
#endif

#if (CONFIG_DW1000_TAG)
    m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_TX_INIT;
    while (1) {
        volatile union DW1000_REG_SYS_STATUS *sys_status = &m_dw1000_ctx.sys_status;
        switch (m_dw1000_ctx.ads_twr_state) {
        case DW1000_ADS_TWR_STATE_TX_INIT:
        {
            sleep_ms(1000);
            printf("-> blink %d\n", m_dw1000_ctx.seq_num);
            m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_BLINK;
            break;
        }
        // Discovery phase
        case DW1000_ADS_TWR_STATE_BLINK:
        {
            union ieee_blink_frame_t *tx_frame = (void *)m_dw1000_ctx.tx_buf;
            tx_frame->fctrl        = IEEE_802_15_4_BLINK_CCP_64;
            tx_frame->seq_num      = ++m_dw1000_ctx.seq_num;
            tx_frame->long_address = m_dw1000_ctx.my_addr;

            printf("-> init wait %d\n", m_dw1000_ctx.seq_num);
            m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_INIT_WAIT;
            dw1000_transmit_message(tx_frame, sizeof(*tx_frame), true);
            break;
        }
        case DW1000_ADS_TWR_STATE_INIT_WAIT:
        {
            if (sys_status->ofs_00.rxfcg)
            {
                sys_status->ofs_00.value = 0;

                union DW1000_REG_RX_FINFO rx_finfo;
                if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_FINFO, &rx_finfo, sizeof(rx_finfo), NULL))
                    goto err;
                printf("rxflen:%d\n", rx_finfo.rxflen);
                union ieee_rng_request_frame_t *rx_frame = (void *)m_dw1000_ctx.rx_buf;
                if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_BUFFER, rx_frame, rx_finfo.rxflen, NULL))
                    goto err;
                print_buf(rx_frame, rx_finfo.rxflen, "rng init frame:\n");

                if ((rx_frame->fctrl == IEEE_802_15_4_FCTRL_RANGE_16) &&
                    ((m_dw1000_ctx.seq_num + 1) == rx_frame->seq_num) &&
                    (rx_frame->code == DW1000_TWR_CODE_RNG_INIT) &&
                    (rx_frame->dst_addr == m_dw1000_ctx.my_addr))
                {
                    m_dw1000_ctx.tar_addr = rx_frame->src_addr;
                    m_dw1000_ctx.seq_num  = rx_frame->seq_num;
                    printf("-> poll %d\n", m_dw1000_ctx.seq_num);
                    m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_POLL;
                }
                else
                {
                    printf("@@ err %d,(%d,%d),%d,%d\n", (rx_frame->fctrl == IEEE_802_15_4_FCTRL_RANGE_16),
                        (m_dw1000_ctx.seq_num + 1), rx_frame->seq_num,
                        (rx_frame->code == DW1000_TWR_CODE_RNG_INIT),
                        (rx_frame->dst_addr == m_dw1000_ctx.my_addr));
                    m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_TX_INIT;
                }
            }
            else if (sys_status->ofs_00.rxrfto)
            {
                sys_status->ofs_00.value = 0;
                m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_TX_INIT;
            }
            break;
        }
        // Ranging phase
        case DW1000_ADS_TWR_STATE_POLL:
        {
            union ieee_rng_request_frame_t *tx_frame = (void *)m_dw1000_ctx.tx_buf;
            tx_frame->fctrl    = IEEE_802_15_4_FCTRL_RANGE_16;
            tx_frame->seq_num  = ++m_dw1000_ctx.seq_num;
            tx_frame->pan_id   = DW1000_PAN_ID;
            tx_frame->dst_addr = m_dw1000_ctx.tar_addr;
            tx_frame->src_addr = m_dw1000_ctx.my_addr;
            tx_frame->code     = DW1000_TWR_CODE_POLL;

            printf("-> response wait %d\n", m_dw1000_ctx.seq_num);
            m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_RESPONSE_WAIT;
            dw1000_transmit_message(tx_frame, sizeof(*tx_frame), true);
            break;
        }
        case DW1000_ADS_TWR_STATE_RESPONSE_WAIT:
        {
            if (sys_status->ofs_00.rxfcg)
            {
                sys_status->ofs_00.value = 0;

                union DW1000_REG_RX_FINFO rx_finfo;
                if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_FINFO, &rx_finfo, sizeof(rx_finfo), NULL))
                    goto err;
                printf("rxflen:%d\n", rx_finfo.rxflen);
                union ieee_rng_request_frame_t *rx_frame = (void *)m_dw1000_ctx.rx_buf;
                if (dw1000_non_indexed_read(spi_cfg, DW1000_RX_BUFFER, rx_frame, rx_finfo.rxflen, NULL))
                    goto err;
                print_buf(rx_frame, rx_finfo.rxflen, "resp frame:\n");

                if ((rx_frame->fctrl == IEEE_802_15_4_FCTRL_RANGE_16) &&
                    ((m_dw1000_ctx.seq_num + 1) == rx_frame->seq_num) &&
                    (rx_frame->code == DW1000_TWR_CODE_RESP) &&
                    (rx_frame->dst_addr == m_dw1000_ctx.my_addr))
                {
                    m_dw1000_ctx.tar_addr = rx_frame->src_addr;
                    m_dw1000_ctx.seq_num  = rx_frame->seq_num;
                    printf("-> final %d\n", m_dw1000_ctx.seq_num);
                    m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_FINAL;
                }
                else
                {
                    printf("@@ err %d,(%d,%d),%d,%d\n", (rx_frame->fctrl == IEEE_802_15_4_FCTRL_RANGE_16),
                        (m_dw1000_ctx.seq_num + 1), rx_frame->seq_num,
                        (rx_frame->code == DW1000_TWR_CODE_RESP),
                        (rx_frame->dst_addr == m_dw1000_ctx.my_addr));
                    m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_TX_INIT;
                }
            }
            else if (sys_status->ofs_00.rxrfto)
            {
                sys_status->ofs_00.value = 0;
                m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_TX_INIT;
            }
            break;
        }
        case DW1000_ADS_TWR_STATE_FINAL:
        {
            union ieee_rng_request_frame_t *tx_frame = (void *)m_dw1000_ctx.tx_buf;
            tx_frame->fctrl    = IEEE_802_15_4_FCTRL_RANGE_16;
            tx_frame->seq_num  = ++m_dw1000_ctx.seq_num;
            tx_frame->pan_id   = DW1000_PAN_ID;
            tx_frame->dst_addr = m_dw1000_ctx.tar_addr;
            tx_frame->src_addr = m_dw1000_ctx.my_addr;
            tx_frame->code     = DW1000_TWR_CODE_FINAL;

            printf("@@ final\n");
            m_dw1000_ctx.ads_twr_state = DW1000_ADS_TWR_STATE_TX_INIT;
            dw1000_transmit_message(tx_frame, sizeof(*tx_frame), false);
            break;
        }
        default:
            hard_assert(0);
        }
    }
#endif

    printf("%s passed.\n", __func__);
    return;
err:
    printf("%s failed.\n", __func__);
    return;
}
