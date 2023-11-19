/*  
*   File: device.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef DEVICE_H
#define DEVICE_H

#include <types.h>

#define DEVICE_BUS_UNDEFINED 0
#define DEVICE_BUS_NONE 1
#define DEVICE_BUS_PCI 2

#define DEVICE_TYPE_UNDEFINED 0
#define DEVICE_TYPE_STORAGE_CONTROLLER 1
#define DEVICE_TYPE_NETWORK_CONTROLLER 2
#define DEVICE_TYPE_DISPLAY_CONTROLLER 3
#define DEVICE_TYPE_MULTIMEDIA_CONTROLLER 4
#define DEVICE_TYPE_MEMORY_CONTROLLER 5
#define DEVICE_TYPE_SERIAL_DEVICE 6
#define DEVICE_TYPE_PARALLEL_DEVICE 7
#define DEVICE_TYPE_GPIB 8
#define DEVICE_TYPE_COMMUNICATION_DEVICE 9
#define DEVICE_TYPE_MODEM 10
#define DEVICE_TYPE_SMART_CARD 11
#define DEVICE_TYPE_SYSTEM_DEVICE 12
#define DEVICE_TYPE_SD_HOST_CONTROLLER 13
#define DEVICE_TYPE_INPUT_CONTROLLER 14
#define DEVICE_TYPE_KEYBOARD 15
#define DEVICE_TYPE_DOCKING_STATION 16
#define DEVICE_TYPE_PROCESSOR 17
#define DEVICE_TYPE_SERIAL_BUS 18
#define DEVICE_TYPE_DRIVE 19

#define DRIVER_TYPE_GENERIC 0
#define DRIVER_TYPE_PCI 1

typedef struct _device {
    char* name;
    uint16_t bus;
    uint16_t type;
    void* data;
    struct _device_driver* driver;
} device_t;

void device_init();
void device_add(device_t* device);
size_t device_get_device_count();
device_t device_get_device(size_t i);
device_t* new_device();

#endif //DEVICE_H
