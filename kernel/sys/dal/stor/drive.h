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

#define DRIVE_IF_UNDEFINED 0
#define DRIVE_IF_IDE 1
#define DRIVE_IF_AHCI 2

#define DRIVE_TYPE_UNDEFINED 0
#define DRIVE_TYPE_DISK 1
#define DRIVE_TYPE_OPTICAL 2
#define DRIVE_TYPE_FLOPPY 3

#define PART_ATTRIB_REQUIRED 1
#define PART_ATTRIB_OS (1 << 1)

#include <types.h>
#include <sys/dal/dal.h>
#include <cpu/smp/spinlock.h>

typedef struct _partition {
    uint64_t startLBA;
    uint64_t endLBA;
    uint64_t attribs;
    bool isSystemPartition;
    size_t size;

    //automatically set by the DAL. should not be touched
    //by drivers.
    bool _valid;
} partition_t;

typedef struct _drive {
    char* name;
    int interface;
    bool isSystemDrive;
    uint8_t type;
    size_t size;
    size_t blockSize;

    void (*read)(struct _drive* self, uint64_t startLBA, size_t blocks, void* buf);
    void (*write)(struct _drive* self, uint64_t startLBA, size_t blocks, void* buf);

    //automatically set by the DAL. should not be touched
    //by drivers.
    bool _valid;
    size_t partitionCount;
    partition_t partitions[MAX_PARTITIONS];

    spinlock_t lock;

    void* context;
} drive_t;

drive_t* drive_add(drive_t drive);
void drive_remove(drive_t* drive);
drive_t* drive_get_all();

#endif //DRIVE_H