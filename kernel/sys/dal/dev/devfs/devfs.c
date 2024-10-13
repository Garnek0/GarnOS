#include "devfs.h"
#include "garn/dal/dal.h"
#include "garn/dal/device-types.h"
#include "garn/ds/list.h"
#include <garn/kstdio.h>
#include <garn/kerrno.h>
#include <sys/term/term-internals.h>
#include <garn/fal/vnode.h>
#include <garn/mm.h>

static vnode_operations_t devfsVnodeOps = {
	.vn_read = NULL,
	.vn_write = NULL,
	.vn_ioctl = NULL,
	.vn_inactive = NULL,
	.vn_lookup = devfs_lookup,
	.vn_mkdir = NULL,
	.vn_rmdir = NULL,
	.vn_readdir = devfs_readdir
};

static vfs_operations_t devfsVFSOps = {
	.vfs_statfs = NULL
};

int devfs_init(const char* mount){
	vfs_t* devfs = kmalloc(sizeof(vfs_t));	
	memset(devfs, 0, sizeof(vfs_t));

	memcpy(devfs->name, "dev", 4);
	memcpy(devfs->type, FILESYS_TYPE_DEVFS, strlen(FILESYS_TYPE_DEVFS)+1);
	devfs->fsops = &devfsVFSOps;

	devfs->rootVnode = vnode_new(devfs, &devfsVnodeOps);

	devfs->rootVnode->flags = 0;
	devfs->rootVnode->type = V_DIR;
	devfs->rootVnode->size = 0;
	devfs->rootVnode->fsData = NULL;

	//TEMPORARY
	
	static device_t tty0;
	tty0.name = "tty0";
	tty0.privateData = tty0.driverData = NULL;
	tty0.bus = DEVICE_BUS_NONE;
	tty0.node = NULL;
	tty0.type = DEVICE_TYPE_SOFTWARE_DEVICE;
	tty0.category = DEVICE_CAT_CHAR;
	tty0.idList = NULL;
	tty0.major = 4;
	tty0.minor = 0;
	tty0.devOps = &termVnodeOps;

	device_add(&tty0);

	return vfs_mount(devfs, mount, 0);
}

vnode_t* devfs_lookup(vnode_t* self, const char* name){
	if(!self || self->type != V_DIR){
		kerrno = ENOTDIR;
		return NULL;
	}

	char fullname[256];

	memcpy(fullname, self->filename, strlen(self->filename)+1);
 	memcpy(&fullname[strlen(self->filename)], name, strlen(name)+1);	

	vnode_t* listEntry = vnode_list_search_by_filename(self->vfs, fullname);
	if(listEntry){
		return listEntry;
	}

	char lookFor[256];
	memcpy(lookFor, name, strlen(name)+1);

	if(lookFor[strlen(lookFor)-1] == '/') lookFor[strlen(lookFor)-1] = 0;

	for(size_t i = 0; i < device_get_device_count(); i++){
		device_t* dev = device_get_device(i);

		if(dev->category == DEVICE_CAT_GENERIC) continue;

		if(!strcmp(lookFor, dev->name)){
			vnode_t* vnode = vnode_new(self->vfs, dev->devOps);

			if(dev->category == DEVICE_CAT_CHAR) vnode->type = V_CHR;
			else vnode->type = V_BLK;

			vnode->size = 0;

			// The device struct address may be needed later.
			vnode->fsData = (void*)dev;

			return vnode;
		}
	}

	kerrno = ENOENT;
	return NULL;
}

ssize_t devfs_readdir(vnode_t* self, size_t count, void* buf, size_t offset){
	ssize_t bytesRead = 0;
	size_t trueOffset = 0;

	dirent_t* dirent = (dirent_t*)buf;

	for(size_t i = 0; i < device_get_device_count(); i++){
		device_t* dev = device_get_device(i);

		if(dev->category == DEVICE_CAT_GENERIC) continue;

		trueOffset += sizeof(dirent_t) + strlen(dev->name);

		if(trueOffset <= offset) continue;
		if(bytesRead + sizeof(dirent_t) + strlen(dev->name) > count) return bytesRead;

		dirent->offset = trueOffset;
		dirent->inode = 0;

		if(dev->category == DEVICE_CAT_CHAR) dirent->type = DT_CHR;
		else dirent->type = DT_BLK;

		dirent->reclen = sizeof(dirent_t) + strlen(dev->name);
		memcpy(dirent->name, dev->name, strlen(dev->name)+1);

		bytesRead += dirent->reclen;

		dirent = (dirent_t*)((uint64_t)dirent + dirent->reclen);
	}	

	return bytesRead;
}

