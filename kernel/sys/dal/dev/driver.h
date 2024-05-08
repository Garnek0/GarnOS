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

struct _device;

typedef uint64_t device_id_t;

typedef struct _device_driver {
    bool (*probe)(struct _device* device);
    bool (*attach)(struct _device* device);
    bool (*remove)(struct _device* device);
    void* data;
} device_driver_t;

typedef struct _driver_node {
    device_driver_t* driver;
    device_id_t* ids;
    bool loaded;
    char* path;
} driver_node_t;

void driver_init();
void device_driver_add(driver_node_t* driver);
bool device_driver_attach(struct _device* device);
int device_driver_unregister_node(driver_node_t* node);
int device_driver_unregister(const char* path);
int device_driver_register(const char* path);
int device_driver_autoreg(const char* path);
size_t device_driver_get_driver_count();
device_driver_t device_driver_get_driver(size_t i);

#endif //DRIVER_H