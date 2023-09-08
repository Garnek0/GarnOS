/*  
*   File: bootloader.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef BOOTLOADER_IF_H
#define BOOTLOADER_IF_H

#include <types.h>

void* bl_get_kernel_file_address();
uint64_t bl_get_kernel_file_size();
uint64_t bl_get_kernel_phys_base();
uint64_t bl_get_kernel_virt_base();
uint64_t bl_get_hhdm_offset();

#endif //BOOTLOADER_IF_H