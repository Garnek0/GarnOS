/*  
*   File: device.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef DEVICE_H
#define DEVICE_H

#include <types.h>
#include <sys/dal/dal.h>

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

#define DEVICE_ID_CLASS(x) ((x & 0xFF00000000000000) >> 56)
#define DEVICE_ID_CLASS_NONE 0

//PS/2 Class

#define DEVICE_ID_CLASS_PS2 0x01ull

#define DEVICE_CREATE_ID_PS2 (DEVICE_ID_CLASS_PS2 << 56)

//PCI Class

#define DEVICE_ID_CLASS_PCI 0x02ull

#define DEVICE_ID_PCI_VENDOR(x) ((x & 0x00FFFF0000000000) >> 40)
#define DEVICE_ID_PCI_VENDOR_ANY 0xFFFF
#define DEVICE_ID_PCI_DEVICE(x) ((x & 0x000000FFFF000000) >> 24)
#define DEVICE_ID_PCI_DEVICE_ANY 0xFFFF
#define DEVICE_ID_PCI_CLASS(x) ((x & 0x0000000000FF0000) >> 16)
#define DEVICE_ID_PCI_SUBCLASS(x) ((x & 0x000000000000FF00) >> 8)

#define DEVICE_CREATE_ID_PCI(vid, did, cls, scls) ((DEVICE_ID_CLASS_PCI << 56) | ((uint64_t)vid << 40) | ((uint64_t)did << 24) | ((uint64_t)cls << 16) | ((uint64_t)scls << 8))
//...

typedef uint64_t device_id_t;

typedef struct _device {
    char* name;
    uint16_t bus;
    uint16_t type;
    void* data;
    device_id_t id;
    struct _driver_node* node;
} device_t;

void device_init();
void device_add(struct _device* device);
size_t device_get_device_count();
device_t device_get_device(size_t i);
bool device_attach_to_driver(struct _driver_node* node);

#endif //DEVICE_H
