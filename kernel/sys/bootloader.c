/*  
*   File: bootloader.c
*
*   Author: Garnek
*   
*   Description: Bootloader related stuff.
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "bootloader.h"
#include <limine.h>

static volatile struct limine_kernel_file_request kernel_file_request = {
    .id = LIMINE_KERNEL_FILE_REQUEST,
    .revision = 0
};

static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

void* bl_get_kernel_file_address(){
    return kernel_file_request.response->kernel_file->address;
}

uint64_t bl_get_kernel_file_size(){
    return kernel_file_request.response->kernel_file->size;
}

uint64_t bl_get_kernel_phys_base(){
    return kernel_address_request.response->physical_base;
}

uint64_t bl_get_kernel_virt_base(){
    return kernel_address_request.response->virtual_base;
}

uint64_t bl_get_hhdm_offset(){
    return hhdm_request.response->offset;
}