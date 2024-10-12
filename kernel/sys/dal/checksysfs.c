#include <sys/dal/dal-internals.h>
#include <garn/fal/vnode.h>
#include <garn/fal/vfs.h>
#include <garn/kstdio.h>

bool checksysfs_check(){
    vfs_t* sysfs = vfs_get_by_fid(0);
    if(!sysfs) return false;
    if(!sysfs->drive) return false;
    if(!sysfs->drive->partitions[sysfs->partition].isSystemPartition){
        klog("System FS is not Present!\n", KLOG_FAILED, "checksysfs");
        return false;
    }

    klog("System FS is Present.\n", KLOG_OK, "checksysfs");

    return true;
}
