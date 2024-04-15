/*  
*   File: filesys.c
*
*   Author: Garnek
*   
*   Description: Filesystem Abstraction
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "filesys.h"
#include <cpu/multiproc/spinlock.h>
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>

filesys_t filesystems[MAX_FILESYSTEMS];
size_t nextAvailFSIndex;

static size_t _filesys_get_next_avail_index(){
    for(size_t i = 0; i < MAX_FILESYSTEMS; i++){
        if(!filesystems[i]._valid) return i;
    }

    panic("Too many filesystems mounted! Limit of %d Filesystems exceeded!", MAX_FILESYSTEMS);
}

filesys_t* filesys_mount(filesys_t filesys){
    filesys._valid = true;
    
    filesys_t* fsAddr;

    if(filesys.drive && filesys.drive->partitions[filesys.partition].isSystemPartition){
        filesystems[0] = filesys;
        filesystems[0].mountNumber = 0;
        fsAddr = &filesystems[0];

        klog("FAL: Mounted system FS %d:/ (%s).\n", KLOG_OK, filesystems[0].mountNumber, filesystems[0].name);

        if(device_driver_autoreg("0:/autoreg.txt") != 0){
            panic("autoreg.txt not found on system fs!");
        }

        return fsAddr;
    }

    filesystems[nextAvailFSIndex] = filesys;
    filesystems[nextAvailFSIndex].mountNumber = nextAvailFSIndex;
    fsAddr = &filesystems[nextAvailFSIndex];

    klog("FAL: Mounted FS %d:/ (%s).\n", KLOG_OK, filesystems[nextAvailFSIndex].mountNumber, filesystems[nextAvailFSIndex].name);

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

void filesys_set_name(filesys_t* fs, char* name){
    size_t nameSize = strlen(name);
    if(nameSize > 32) nameSize = 32;

    memcpy(fs->name, name, nameSize);
    fs->name[nameSize] = 0;
}