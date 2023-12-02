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

#define DRIVER_TYPE_UNDEFINED 0
#define DRIVER_TYPE_PS2 1
#define DRIVER_TYPE_PCI 2
//...

typedef struct _device_driver {
    int type;
    bool (*probe)(struct _device* device);
    bool (*attach)(struct _device* device);
    bool (*remove)(struct _device* device);
    void* data;
} device_driver_t;

typedef struct _driver_node {
    device_driver_t* driver;
    bool loaded;
    char* path;
} driver_node_t;

void driver_init();
void device_driver_add(driver_node_t* driver);
bool device_driver_attach(device_t* device);
size_t device_driver_get_driver_count();
device_driver_t device_get_driver(size_t i);

#endif //DRIVER_H