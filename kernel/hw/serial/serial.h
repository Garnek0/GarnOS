/*  
*   File: serial.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef SERIAL_H
#define SERIAL_H

#include <types.h>

#define COM_PORT 0x3F8

#define COM_DATA COM_PORT
#define COM_INT COM_PORT+1
#define COM_DIVISOR_LSB COM_PORT
#define COM_DIVISOR_MSB COM_PORT+1
#define COM_FIFO_CTRL COM_PORT+2
#define COM_LINE_CONTROL COM_PORT+3
#define COM_MODEM_CTRL COM_PORT+4
#define COM_LINE_STATUS COM_PORT+5
#define COM_SCRATCH COM_PORT+7

int serial_init();
void serial_write(uint8_t data);
void serial_log(const char* str);

inline void serial_enable_logs();
inline void serial_disable_logs();

#endif //SERIAL_H