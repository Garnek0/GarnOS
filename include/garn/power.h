/*  
*   File: power.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef POWER_H
#define POWER_H

#include <garn/types.h>

typedef struct _power {
    int (*shutdown)();
    int (*restart)();
    int (*suspend)();
} power_t;

int power_shutdown();
int power_restart();
int power_suspend();

void power_set_shutdown(void* func);
void power_set_restart(void* func);
void power_set_suspend(void* func);

#endif //POWER_H