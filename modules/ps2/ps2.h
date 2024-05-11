/*  
*   File: ps2.h
*
*   Author: Garnek
*   
*   Description: Defines for easy access to the PS/2 Controller and Devices
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef PS2_MODULE_H
#define PS2_MODULE_H

#include <hw/ports.h>
#include <kstdio.h>
#include <sys/dal/dal.h>

#define TIMEOUT 100000

#define PS2_DATA 0x60
#define PS2_COMMAND 0x64

#define PS2_STATUS_READRDY 1
#define PS2_STATUS_WRITERDY (1 << 1)
#define PS2_STATUS_TIMEOUT_ERR (1 << 6)
#define PS2_STATUS_PARITY_ERR (1 << 7)

#define PS2_CONFIG_PORT1_IEN 1
#define PS2_CONFIG_PORT2_IEN (1 << 1)
#define PS2_CONFIG_PORT1_CLK (1 << 4)
#define PS2_CONFIG_PORT2_CLK (1 << 5)
#define PS2_CONFIG_TRANSLATION (1 << 6)

#define PS2_COMMAND_GET_CONFIG 0x20
#define PS2_COMMAND_SET_CONFIG 0x60

#define PS2_COMMAND_DISABLE_PORT2 0xA7
#define PS2_COMMAND_ENABLE_PORT2 0xA8
#define PS2_COMMAND_TEST_PORT2 0xA9

#define PS2_COMMAND_SELFTEST 0xAA
#define PS2_COMMAND_DIAG_DUMP 0xAC

#define PS2_COMMAND_TEST_PORT1 0xAB
#define PS2_COMMAND_DISABLE_PORT1 0xAD
#define PS2_COMMAND_ENABLE_PORT1 0xAE

#define PS2_COMMAND_RD_CONTROLLER_INPUT 0xC0
#define PS2_COMMAND_RD_CONTROLLER_OUTPUT 0xD0

#define PS2_COMMAND_WR_CONTROLLER_OUTPUT 0xD1
#define PS2_COMMAND_WR_PORT1_OUTPUT 0xD2
#define PS2_COMMAND_WR_PORT2_OUTPUT 0xD3
#define PS2_COMMAND_WR_PORT2_INPUT 0xD4

#define PS2_DEVICE_RESET 0xFF

#define LEFT_SHIFT 0x2A
#define LEFT_CTRL 0x1D
#define RIGHT_SHIFT 0x36
#define RIGHT_CTRL 0x1D
#define ENTER 0x1C
#define BACKSPACE 0x0E
#define CAPSLOCK 0x3A
#define EX_SCANCODE 0xE0y

#endif //PS2_MODULE_H