/*  
*   Module: ahci.sys
*
*   File: ahci-atapi-io.c
*
*   Module Author: Garnek
*   
*   Module Description: AHCI ATAPI I/O Control
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "ahci.h"

int ahci_atapi_read(drive_t* drive, size_t startLBA, size_t blocks, void* buf){
    return 0;
}

int ahci_atapi_write(drive_t* drive, size_t startLBA, size_t blocks, void* buf){
    return 0;
}
