/*  
*   File: drive.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef DRIVE_H
#define DRIVE_H

#define MAX_PARTITIONS 128
#define MAX_DRIVES 256

#define DRIVE_PROGIF_IDE 0
#define DRIVE_PROGIF_AHCI 1

#include <types.h>
#include <sys/device.h>

typedef struct _partition {
    bool avail;
    uint64_t startLBA;
    uint64_t endLBA;
    size_t size;
} partition_t;

typedef struct _drive {
    bool _avail;
    char* name;
    int interface;
    size_t size;

    void (*read)(struct _drive* self, void* buf, uint64_t startLBA, size_t size);
    void (*write)(struct _drive* self, void* buf, uint64_t startLBA, size_t size);

    device_t* _device;

    size_t partitionCount;
    partition_t partitions[MAX_PARTITIONS];

    void* context;
} drive_t;

drive_t* drive_add(drive_t drive);
void drive_remove(drive_t* drive);
drive_t* drive_get_all();

#endif //DRIVE_H