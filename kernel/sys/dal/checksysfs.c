/*  
*   File: checksysfs.c
*
*   Author: Garnek
*   
*   Description: Checks whether the system fs is mounted or not
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "checksysfs.h"
#include <sys/fal/fal.h>
#include <kstdio.h>

bool checksysfs_check(){
    filesys_t* sysfs = filesys_get(0);
    if(!sysfs) return false;
    if(!sysfs->_valid) return false;
    if(!sysfs->drive) return false;
    if(!sysfs->drive->partitions[sysfs->partition].isSystemPartition){
        klog("System FS is not Present!\n", KLOG_FAILED, "checksysfs");
        return false;
    }

    klog("System FS is Present.\n", KLOG_OK, "checksysfs");

    return true;
}