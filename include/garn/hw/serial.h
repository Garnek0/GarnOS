/*  
*   File: serial.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef SERIAL_H
#define SERIAL_H

#include <garn/types.h>

void serial_write(uint8_t data);
void serial_log(const char* str);

void serial_enable_logs();
void serial_disable_logs();

#endif //SERIAL_H