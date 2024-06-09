/*  
*   File: time.c
*
*   Author: Garnek
*
*   Description: System time
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <garn/time.h>

systime_t systemTime;

void time_set(systime_t time){
    systemTime = time;
}

systime_t time_get(){
    return systemTime;
}

