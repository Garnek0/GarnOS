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

vfs_t* rootVFS;
vfs_t* lastVFS;

static int _vfs_gen_fid(){
	static size_t fid = 0;
	return fid++;
}

int vfs_mount(vfs_t* vfs){

	// If vfs is the system partition FS, then we need to make sure it has fid 0
	if(vfs->drive && vfs->drive->partitions[vfs->partition].isSystemPartition){
        vfs->fid = 0;

		//FIXME: This has the same problem as vfs_unmount()
		vfs->next = rootVFS->next;
		if(rootVFS == lastVFS) lastVFS = vfs;
		if(!rootVFS) kmfree(rootVFS);
		rootVFS = vfs;

        klog("Mounted system FS \"%s\" (fid: 0).\n", KLOG_OK, "FAL", vfs->name);

        if(device_driver_autoreg("0:/drv/autoreg.txt") != 0){
            panic("autoreg.txt not found on system fs!", "FAL");
        }

        return 0;
    }

	if(!rootVFS){
		rootVFS = lastVFS = vfs;
		rootVFS->next = NULL;
	} else {
		lastVFS->next = vfs;
	}

    vfs->fid = _vfs_gen_fid();

    klog("Mounted FS \"%s\" (fid: %d).\n", KLOG_OK, "FAL", vfs->name, vfs->fid);

    return 0;
}

int vfs_unmount(vfs_t* vfs){
	//FIXME: This just destroys the vfs. Maybe not the safest approach.
	vfs_t* prev = NULL;
	for(vfs_t* i = rootVFS; i; i = i->next){
		if(i == vfs){
			vfs_t* next = i->next;
			if(prev){
				lock(prev->lock, {
					prev->next = i->next;
				});
			} else {
				// Something really bad happened if there's an attempt to unmount the ROOT VFS!!
				klog("Attempt to unmount the root VFS!\n", KLOG_WARNING, "FAL");
				return 1;
			}
			kmfree(i);
			return 0;
		}
		prev = i;
	}

	return 1;	
}

vfs_t* vfs_get_by_fid(size_t fid){
    for(vfs_t* i = rootVFS; i; i = i->next){
		if(i->fid == fid){
			return i;
		}
	}

	return NULL;
}

vfs_t* vfs_get_root(){
	return rootVFS;
}
