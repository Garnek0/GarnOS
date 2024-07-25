/*  
*   File: vfs.c
*
*   Author: Garnek
*   
*   Description: Virtual Filesystem Layer
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fal-internals.h"

#include <garn/fal/vfs.h>
#include <garn/spinlock.h>
#include <garn/mm.h>
#include <garn/kstdio.h>

vfs_t vfs[MAX_VFS];
size_t nextAvailFSIndex;

static size_t _vfs_get_next_avail_index(){
    for(size_t i = 0; i < MAX_VFS; i++){
        if(!vfs[i]._valid) return i;
    }

    panic("Too many filesystems mounted! Limit of %d Filesystems exceeded!", "FAL", MAX_VFS);
    return (size_t)-1;
}

vfs_t* vfs_mount(vfs_t filesys){
    filesys._valid = true;
    
    vfs_t* fsAddr;

    filesys.lock = 0;

    if(filesys.drive && filesys.drive->partitions[filesys.partition].isSystemPartition){
        vfs[0] = filesys;
        vfs[0].mountNumber = 0;
        fsAddr = &vfs[0];

        klog("Mounted system FS %d:/ (%s).\n", KLOG_OK, "FAL", vfs[0].mountNumber, vfs[0].name);

        if(device_driver_autoreg("0:/drv/autoreg.txt") != 0){
            panic("autoreg.txt not found on system fs!", "FAL");
        }

        return fsAddr;
    }

	vfs[nextAvailFSIndex] = filesys;
    vfs[nextAvailFSIndex].mountNumber = nextAvailFSIndex;
    fsAddr = &vfs[nextAvailFSIndex];

    klog("Mounted FS %d:/ (%s).\n", KLOG_OK, "FAL", vfs[nextAvailFSIndex].mountNumber, vfs[nextAvailFSIndex].name);

    nextAvailFSIndex = _vfs_get_next_avail_index();

    return fsAddr;
}

void vfs_unmount(vfs_t* filesys){
    lock(filesys->lock, {
        filesys->_valid = false;
        nextAvailFSIndex = _vfs_get_next_avail_index();
    });
}

vfs_t* vfs_get(size_t index){
    return &vfs[index];
}

vfs_t* vfs_get_all(){
    return vfs;
}
