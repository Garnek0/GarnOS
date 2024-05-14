/*  
*   File: timer.c
*
*   Author: Garnek
*   
*   Description: Simple timer interface.
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <garn/timer.h>
#include <hw/pit/pit.h>

//for now these are just wrappers for PIT functions

void ksleep(uint64_t ms){
    pit_sleep(ms);
}

uint64_t timer_get_ticks(){
    return pit_get_ticks();
}