/*  
*   File: filesys.c
*
*   Author: Garnek
*   
*   Description: Filesystem Abstraction
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fal-internals.h"

#include <garn/fal/filesys.h>
#include <garn/spinlock.h>
#include <garn/mm.h>
#include <garn/kstdio.h>

filesys_t filesystems[MAX_FILESYSTEMS];
size_t nextAvailFSIndex;

static size_t _filesys_get_next_avail_index(){
    for(size_t i = 0; i < MAX_FILESYSTEMS; i++){
        if(!filesystems[i]._valid) return i;
    }

    panic("Too many filesystems mounted! Limit of %d Filesystems exceeded!", "FAL", MAX_FILESYSTEMS);
    return (size_t)-1;
}

filesys_t* filesys_mount(filesys_t filesys){
    filesys._valid = true;
    
    filesys_t* fsAddr;

    if(filesys.drive && filesys.drive->partitions[filesys.partition].isSystemPartition){
        filesystems[0] = filesys;
        filesystems[0].mountNumber = 0;
        fsAddr = &filesystems[0];

        klog("Mounted system FS %d:/ (%s).\n", KLOG_OK, "FAL", filesystems[0].mountNumber, filesystems[0].name);

        if(device_driver_autoreg("0:/autoreg.txt") != 0){
            panic("autoreg.txt not found on system fs!", "FAL");
        }

        return fsAddr;
    }

    filesystems[nextAvailFSIndex] = filesys;
    filesystems[nextAvailFSIndex].mountNumber = nextAvailFSIndex;
    fsAddr = &filesystems[nextAvailFSIndex];

    klog("Mounted FS %d:/ (%s).\n", KLOG_OK, "FAL", filesystems[nextAvailFSIndex].mountNumber, filesystems[nextAvailFSIndex].name);

    nextAvailFSIndex = _filesys_get_next_avail_index();

    return fsAddr;
}

void filesys_unmount(filesys_t* filesys){
    lock(filesys->lock, {
        filesys->_valid = false;
        nextAvailFSIndex = _filesys_get_next_avail_index();
    });
}

filesys_t* filesys_get(size_t index){
    return &filesystems[index];
}

filesys_t* filesys_get_all(){
    return filesystems;
}