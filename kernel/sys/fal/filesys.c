/*  
*   File: filesys.c
*
*   Author: Garnek
*   
*   Description: Filesystem Abstraction
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "filesys.h"
#include <cpu/smp/spinlock.h>
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>

filesys_t filesystems[MAX_FILESYSTEMS];
size_t nextAvailFSIndex;

spinlock_t filesysLock;

static size_t _filesys_get_next_avail_index(){
    for(size_t i = 0; i < MAX_FILESYSTEMS; i++){
        if(!filesystems[i]._valid) return i;
    }

    panic("Too many filesystems mounted! Limit of %d Filesystems exceeded!", MAX_FILESYSTEMS);
}

filesys_t* filesys_mount(filesys_t filesys){
    filesys._valid = true;
    
    filesys_t* fsAddr;

    lock(filesysLock, {
        if(filesys.drive && filesys.drive->partitions[filesys.partition].isSystemPartition){
            filesystems[0] = filesys;
            filesystems[0].mountNumber = 0;
            fsAddr = &filesystems[0];

            klog("FAL: Mounted system partition.\n", KLOG_OK);

            if(device_driver_autoreg("0:/autoreg.txt") != 0){
                panic("autoreg.txt not found on system fs!");
            }

            releaseLock(&filesysLock);
            return fsAddr;
        }

        filesystems[nextAvailFSIndex] = filesys;
        filesystems[nextAvailFSIndex].mountNumber = nextAvailFSIndex;
        fsAddr = &filesystems[nextAvailFSIndex];

        nextAvailFSIndex = _filesys_get_next_avail_index();
    });

    return fsAddr;
}

void filesys_unmount(filesys_t* filesys){
    lock(filesysLock, {
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

void filesys_set_name(filesys_t* fs, char* name){
    size_t nameSize = strlen(name);
    if(nameSize > 32) nameSize = 32;

    memcpy(fs->name, name, nameSize);
    fs->name[nameSize] = 0;
}