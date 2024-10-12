#include "fal-internals.h"

#include <garn/fal/vfs.h>
#include <garn/spinlock.h>
#include <garn/mm.h>
#include <garn/kstdio.h>
#include <garn/kerrno.h>
#include <sys/dal/dal-internals.h>

/*TODO:
- [ ] Make sure mounting filesystems works as expected
- [ ] Fix the FIXMEs
*/

vfs_t* rootVFS;
vfs_t* lastVFS;

static size_t _vfs_gen_fid(){
	static size_t fid = 0;
	return fid++;
}

int vfs_mount(vfs_t* vfs, const char* mnt, uint32_t flags){
	// If vfs is the system partition FS, then we need to make sure it has fid 0 and is mounted as root
	if(vfs->drive && vfs->drive->partitions[vfs->partition].isSystemPartition){
        vfs->fid = 0;
		vfs->mountFlags = 0;

		//FIXME: This has the same problem as vfs_unmount()
		vfs->next = rootVFS->next;
		if(rootVFS == lastVFS) lastVFS = vfs;
		if(!rootVFS) kmfree(rootVFS);
		rootVFS = vfs;
		vnode_list_reset();

        klog("Mounted system FS \"%s\" (fid: 0).\n", KLOG_OK, "FAL", vfs->name);

		//Mount Linux special filesystems now
		
		if(devfs_init("/dev/") != 0){
			panic("Failed to mount devfs!", "FAL");
		} else {
			klog("Mounted devfs at /dev\n", KLOG_OK, "FAL");
		}

        if(device_driver_autoreg("/drv/autoreg.txt") != 0){
            panic("autoreg.txt not found on system fs!", "FAL");
        }

        return 0;
    }

	vfs->mountFlags = flags;

	if(!rootVFS){
		rootVFS = lastVFS = vfs;
		rootVFS->next = NULL;
	} else {
		if(!strcmp(mnt, "/")){
			return -1;
		}

		vnode_t* mntVnode = vnode_open(mnt, O_DIRECTORY, 0);

		if(!mntVnode){
			return -kerrno;
		}

		lastVFS->next = vfs;
		lastVFS = vfs;
		mntVnode->vfsMountedHere = vfs;
		vfs->vnodeCovered = mntVnode;
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
