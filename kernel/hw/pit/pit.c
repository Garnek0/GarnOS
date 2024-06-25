/*  
*   File: pit.c
*
*   Author: Garnek
*   
*   Description: Programmable Interval Timer
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "pit.h"
#include <garn/irq.h>
#include <garn/irq.h>
#include <garn/arch.h>
#include <garn/spinlock.h>
#include <process/sched/sched.h>
#include <garn/kstdio.h>
#include <garn/timer.h>

void pit_init(){
    
}