#ifndef VNODE_H
#define VNODE_H

#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      (1 << 1)
#define O_CREAT     (1 << 6)
#define O_EXCL      (1 << 7)
#define O_TRUNC     (1 << 9)
#define O_APPEND    (1 << 10)
#define O_NONBLOCK  (1 << 11)
#define O_DIRECTORY (1 << 16)

#define S_ISUID 0x0800
#define S_ISGID 0x0400
#define S_ISVTX 0x0200
#define S_IRWXU 0x01C0
#define S_IRUSR 0x0100
#define S_IWUSR 0x0080
#define S_IXUSR 0x0040
#define S_IRWXG 0x0038
#define S_IRGRP 0x0020
#define S_IWGRP 0x0010
#define S_IXGRP 0x0008
#define S_IRWXO 0x0007
#define S_IROTH 0x0004
#define S_IWOTH 0x0002
#define S_IXOTH 0x0001

#define V_NON 0
#define V_REG 1
#define V_DIR 2
#define V_BLK 3
#define V_CHR 4
#define V_LNK 5
#define V_SOCK 6
#define V_BAD 7

#define DT_UNKNOWN 0
#define DT_REG 1
#define DT_DIR 2

#include <garn/types.h>
#include <garn/spinlock.h>
#include <garn/fal/vfs.h>
#include <garn/time.h>

struct _vnode;

typedef struct _dirent {
    uint32_t inode;
    uint32_t offset; 
    uint16_t reclen;
	uint8_t type;
    char name[1];
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
dirent_t;

typedef struct _vnode_operations {
	int (*vn_read)(struct _vnode* self, size_t count, void* buf, size_t offset);
	int (*vn_write)(struct _vnode* self, size_t count, void* buf, size_t offset);
	int (*vn_ioctl)(struct _vnode* self, int op, ...);
	//vn_select()
	//...
	int (*vn_inactive)(struct _vnode* self);

	struct _vnode* (*vn_mkdir)(struct _vnode* self, const char* name);
	int (*vn_rmdir)(struct _vnode* self, const char* name);
	struct _vnode* (*vn_lookup)(struct _vnode* self, const char* name);
	ssize_t (*vn_readdir)(struct _vnode* self, size_t count, void* buf, size_t offset);
} vnode_operations_t;

typedef struct _vnode {
	char filename[256];
	int mode;
	int flags;
	int type;

	size_t size; //file size
	
	timespec64_t lastAccessedTime;
	timespec64_t lastModifiedTime; //File data modifications
	timespec64_t lastChangedTime; //Inode changes

	int refCount;
	spinlock_t lock;

	struct _vfs* vfsMountedHere;
	struct _vfs* vfs;

	struct _vnode_operations* vnodeops;

	void* fsData;

	struct _vnode* next;
	struct _vnode* prev;
} vnode_t;

void vnode_list_add(vnode_t* vnode);
void vnode_list_remove(vnode_t* vnode);
vnode_t* vnode_list_search_by_filename(struct _vfs* vfs, const char* filename);
void vnode_list_reset();

vnode_t* vnode_new(struct _vfs* vfs, vnode_operations_t* vnodeops);

//lookup path name
vnode_t* vnode_lookup(const char* path);

//open vnode
vnode_t* vnode_open(const char* path, int flags, int mode);

//close vnode
int vnode_close(vnode_t* vnode);

//read from vnode
ssize_t vnode_read(vnode_t* vnode, size_t size, void* buf, size_t offset);

//write to vnode
ssize_t vnode_write(vnode_t* vnode, size_t size, void* buf, size_t offset);

//read directory entries of a vnode
ssize_t vnode_readdir(vnode_t* vnode, size_t count, void* buf, size_t offset);

#endif //VNODE_H
