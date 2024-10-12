#pragma once

#include <garn/arch.h>
#include <garn/kstdio.h>
#include <garn/dal/dal.h>
#include <garn/module.h>
#include <garn/irq.h>
#include <garn/mm.h>
#include <garn/input.h>

#define TIMEOUT 100000

#define i8042_DATA 0x60
#define i8042_COMMAND 0x64

#define i8042_STATUS_READRDY 1
#define i8042_STATUS_WRITERDY (1 << 1)
#define i8042_STATUS_TIMEOUT_ERR (1 << 6)
#define i8042_STATUS_PARITY_ERR (1 << 7)

#define i8042_CONFIG_PORT1_IEN 1
#define i8042_CONFIG_PORT2_IEN (1 << 1)
#define i8042_CONFIG_PORT1_CLK (1 << 4)
#define i8042_CONFIG_PORT2_CLK (1 << 5)
#define i8042_CONFIG_TRANSLATION (1 << 6)

#define i8042_COMMAND_GET_CONFIG 0x20
#define i8042_COMMAND_SET_CONFIG 0x60

#define i8042_COMMAND_DISABLE_PORT2 0xA7
#define i8042_COMMAND_ENABLE_PORT2 0xA8
#define i8042_COMMAND_TEST_PORT2 0xA9

#define i8042_COMMAND_SELFTEST 0xAA
#define i8042_COMMAND_DIAG_DUMP 0xAC

#define i8042_COMMAND_TEST_PORT1 0xAB
#define i8042_COMMAND_DISABLE_PORT1 0xAD
#define i8042_COMMAND_ENABLE_PORT1 0xAE

#define i8042_COMMAND_RD_CONTROLLER_INPUT 0xC0
#define i8042_COMMAND_RD_CONTROLLER_OUTPUT 0xD0

#define i8042_COMMAND_WR_CONTROLLER_OUTPUT 0xD1
#define i8042_COMMAND_WR_PORT1_OUTPUT 0xD2
#define i8042_COMMAND_WR_PORT2_OUTPUT 0xD3
#define i8042_COMMAND_WR_PORT2_INPUT 0xD4

#define i8042_DEVICE_RESET 0xFF

#define LEFT_SHIFT 0x2A
#define LEFT_CTRL 0x1D
#define RIGHT_SHIFT 0x36
#define RIGHT_CTRL 0x1D
#define ENTER 0x1C
#define BACKSPACE 0x0E
#define CAPSLOCK 0x3A
#define EX_SCANCODE 0xE0

void i8042_write(uint8_t port, uint8_t data);
uint8_t i8042_read(uint8_t port);

void i8042_kb_init();
void i8042_kb_fini();
