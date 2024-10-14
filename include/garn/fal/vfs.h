#pragma once

#include <garn/types.h>
#include <garn/dal/dal.h>
#include <garn/spinlock.h>
#include <garn/fal/vnode.h>

#define FILESYS_TYPE_UNDEFINED "undefined"
#define FILESYS_TYPE_INIT_USTAR "init-ustar"
#define FILESYS_TYPE_DEVFS "devfs"
#define FILESYS_TYPE_PROCFS "procfs"
#define FILESYS_TYPE_FAT12 "fat12"
#define FILESYS_TYPE_FAT16 "fat16"
#define FILESYS_TYPE_FAT32 "fat32"
#define FILESYS_TYPE_EXFAT "exfat"
#define FILESYS_TYPE_EXT "ext"
#define FILESYS_TYPE_EXT2 "ext2"
#define FILESYS_TYPE_EXT3 "ext3"
#define FILESYS_TYPE_EXT4 "ext4"
#define FILESYS_TYPE_ISO9660 "iso9660"

#define MS_RDONLY 1

#define PATH_MAX 4096

struct _vfs; //Do this so that the compiler doesnt complain about incomplete types

typedef struct _statfs {
	uint32_t type; 
	uint64_t bsize; 
	uint64_t totalBlocks;
	uint64_t freeBlocks;
	uint64_t availBlocks;
	uint64_t files;
	uint64_t freeFiles;
	uint64_t fid;
	uint32_t maxNameLength;
	uint32_t unused; // fragment size in Linux, unused in Garn
	uint32_t mountFlags;
} statfs_t;

typedef struct _vfs_operations {
	statfs_t (*vfs_statfs)(struct _vfs* self);
} vfs_operations_t;

typedef struct _vfs {
	char name[32]; //
	char type[32]; //
	uint32_t mountFlags;

	vfs_operations_t* fsops; //

	size_t fid; //

	struct _vnode* vnodeCovered;
	struct _vnode* rootVnode;

	struct _drive* drive; //
	size_t partition; //

	spinlock_t lock; //

	void* context; //

	struct _vfs* next; //
} vfs_t;

int vfs_mount(vfs_t* vfs, const char* mnt, uint32_t flags);
int vfs_unmount(vfs_t* vfs);
vfs_t* vfs_get_by_fid(size_t fid);
vfs_t* vfs_get_root();
bool vfs_system_fs_present();
