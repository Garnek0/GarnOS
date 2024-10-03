/*  
*   File: devfs.c
*
*   Author: Garnek
*   
*   Description: /dev filesystem
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "devfs.h"
#include <garn/kstdio.h>
#include <garn/kerrno.h>
#include <sys/term/term-internals.h>
#include <garn/mm.h>

static const vnode_operations_t devfsVnodeOps = {
	.vn_read = NULL,
	.vn_write = NULL,
	.vn_ioctl = NULL,
	.vn_inactive = NULL,
	.vn_lookup = devfs_lookup,
	.vn_mkdir = NULL,
	.vn_rmdir = NULL,
	.vn_readdir = NULL
};

static const vfs_operations_t devfsVFSOps = {
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

	//TODO: temporary
	if(!strcmp(self->filename, "")){
		if(!strcmp(lookFor, "tty0")){
			vnode_t* vnode = vnode_new(self->vfs, &termVnodeOps);

			vnode->type = V_CHR; //TODO: change this to char dev
			vnode->size = 0;

			return vnode;
		}
	}

	return NULL;
}
