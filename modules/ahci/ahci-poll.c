/*  
*   Module: ahci.sys
*
*   File: ahci-poll.c
*
*   Module Author: Garnek
*   
*   Module Description: AHCI Polling functions
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "ahci.h"

bool ahci_wait_set(volatile uint32_t* reg, uint32_t bit, uint32_t ms){
    for(size_t i = 0; i < ms + 1; i++){
        if(*reg & bit) return true;
        ksleep(1);
    }
    return false;
}

bool ahci_wait_clear(volatile uint32_t* reg, uint32_t bit, uint32_t ms){
    for(size_t i = 0; i < ms + 1; i++){
        if(!(*reg & bit)) return true;
        ksleep(1);
    }
    return false;
}