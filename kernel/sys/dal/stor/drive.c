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
#include <cpu/smp/spinlock.h>
#include <sys/fal/fal.h>

drive_t drives[MAX_DRIVES];
size_t driveCount;
size_t nextAvailDriveIndex;

spinlock_t driveLock;

static size_t _drive_get_next_avail_index(){
    for(size_t i = 0; i < MAX_DRIVES; i++){
        if(!drives[i]._valid) return i;
    }

        panic("Too many drives attached! Limit of %d Drives exceeded!", MAX_DRIVES);
}

drive_t* drive_add(drive_t drive){
    drive._valid = true;
    drive.partitionCount = 0;

    device_t* drvDevice = new_device();
    drive_t* drvAddr;

    lock(driveLock, {
        drvDevice->type = DEVICE_TYPE_DRIVE;
        drvDevice->bus = DEVICE_BUS_NONE;
        drvDevice->driver = NULL;
        drvDevice->name = drive.name;
        drvDevice->data = (void*)&drives[nextAvailDriveIndex];

        drive._device = drvDevice;

        drives[nextAvailDriveIndex] = drive;
        drvAddr = &drives[nextAvailDriveIndex];

        nextAvailDriveIndex = _drive_get_next_avail_index();

        klog("DAL: Added New Drive \"%s\".\n", KLOG_OK, drvAddr->name);

        //search for partitions

        if(drive.type == DRIVE_TYPE_OPTICAL){
            //partitions are ignored for optical media
            drvAddr->partitionCount = 1;
            drvAddr->partitions[0].attribs = 0;
            drvAddr->partitions[0].startLBA = 0;
            drvAddr->partitions[0].endLBA = 0;
            drvAddr->partitions[0].size = 0;
            drvAddr->partitions[0]._valid = true;
        } else if(gpt_validate_drive(drvAddr)){
            if(!gpt_initialise_drive(drvAddr)) klog("DAL: Drive \"%s\" has a corrupt GPT Header or Table!\n", KLOG_FAILED, drvAddr->name);
        } else {
            klog("DAL: Drive \"%s\" not partitioned or partition table unsupported!\n", KLOG_WARNING, drvAddr->name);
        }
        //TODO: Add MBR Partition Table support

        //search for filesystems

        for(size_t i = 0; i < drvAddr->partitionCount; i++){
            if(fat_probe(drvAddr, i)){
                fat_attach(drvAddr, i);
            }
        }
        
    });

    return drvAddr;
}

void drive_remove(drive_t* drive){
    lock(driveLock, {
        drive->_valid = false;
        nextAvailDriveIndex = _drive_get_next_avail_index();
    });
}

drive_t* drive_get_all(){
    return drives;
}