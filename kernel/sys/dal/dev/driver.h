/*  
*   File: driver.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef DRIVER_H
#define DRIVER_H

#include <types.h>
#include "device.h"

typedef struct _device_driver {
    char* name;
    bool (*probe)(struct _device* device);
    bool (*attach)(struct _device* device);
    bool (*remove)(struct _device* device);
    void* data;
} device_driver_t;

void driver_init();
void device_driver_add(device_driver_t* driver);
bool device_driver_attach(device_t* device);
size_t device_driver_get_driver_count();
device_driver_t device_get_driver(size_t i);

#endif //DRIVER_H