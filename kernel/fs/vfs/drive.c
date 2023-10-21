/*  
*   File: drive.c
*
*   Author: Garnek
*   
*   Description: Storage Medium Abstraction (Drive Abstraction)
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "drive.h"
#include <mem/memutil/memutil.h>
#include <mem/mm/kheap.h>
#include <sys/panic.h>

drive_t drives[256];
size_t driveCount;
size_t nextAvailDriveIndex;

static size_t _drive_get_next_avail_index(){
    for(size_t i = 0; i < MAX_DRIVES; i++){
        if(!drives[i]._avail) return i;
    }

    panic("Too many drives attached! Limit of %d Drives exceeded!", MAX_DRIVES);
}

drive_t* drive_add(drive_t drive){
    drive._avail = true;

    device_t* drvDevice = new_device();
    drive_t* drvAddr;

    drvDevice->type = DEVICE_TYPE_DRIVE;
    drvDevice->bus = DEVICE_BUS_NONE;
    drvDevice->driver = NULL;
    drvDevice->name = drive.name;
    drvDevice->data = (void*)&drives[nextAvailDriveIndex];

    drive._device = drvDevice;

    drives[nextAvailDriveIndex] = drive;
    drvAddr = &drives[nextAvailDriveIndex];

    nextAvailDriveIndex = _drive_get_next_avail_index();

    return drvAddr;
}

void drive_remove(drive_t* drive){
    drive->_avail = false;
    nextAvailDriveIndex = _drive_get_next_avail_index();
}

drive_t* drive_get_all(){
    return drives;
}