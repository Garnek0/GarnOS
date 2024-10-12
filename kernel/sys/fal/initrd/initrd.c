#include "initrd.h"
#include <garn/kstdio.h>
#include <garn/panic.h>
#include <sys/bootloader.h>
#include <garn/mm.h>
#include <limine.h>
#include <garn/kerrno.h>

static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

static const vnode_operations_t initrdVnodeOps = {
	.vn_read = initrd_read,
	.vn_write = NULL,
	.vn_ioctl = NULL,
	.vn_inactive = initrd_inactive,
	.vn_lookup = initrd_lookup,
	.vn_mkdir = NULL,
	.vn_rmdir = NULL,
	.vn_readdir = NULL,
};

static const vfs_operations_t initrdVFSOps = {
	.vfs_statfs = initrd_statfs
};

initrd_tar_header_t* initrd;

//this function converts a ustar value (which is stored as octal in ascii)
//into a regular decimal integer
static uint64_t initrd_tar_conv_number(char* str, size_t size){
    uint64_t n = 0;

    for(size_t i = 0; i < size; i++){
        n *= 8;
        n += (uint64_t)(str[i] - '0');
    }
    return n;
}

ssize_t initrd_read(vnode_t* self, size_t size, void* buf, size_t offset){
	initrd_file_fs_data_t* fsData = (initrd_file_fs_data_t*)self->fsData;

    for(size_t i = 0; i < size; i++){
        if(offset >= self->size) return i;
        ((uint8_t*)buf)[i] = ((uint8_t*)fsData->startOffset)[offset];
        offset++;
    }

    return size;
}

int initrd_inactive(vnode_t* vnode){
	kmfree(vnode->fsData);
	kmfree(vnode);

	return 0;
}

vnode_t* initrd_lookup(vnode_t* self, const char* name){
	initrd_tar_header_t* h = initrd;
    uint64_t haddr = (uint64_t)h;

    size_t size;

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

    for(int i = 0; ; i++){
        if(h->filename[0] == 0){
            klog("Couldn't find %s\n", KLOG_FAILED, "initrd", fullname);
            kerrno = ENOENT;
            return NULL;
        } else if (!strcmp(fullname, h->filename)){
            size = (size_t)initrd_tar_conv_number(h->size, 11);

            vnode_t* vnode = vnode_new(self->vfs, &initrdVnodeOps);

            initrd_file_fs_data_t* fsData = kmalloc(sizeof(initrd_file_fs_data_t));

			memcpy(vnode->filename, fullname, 256);
			if(h->typeFlag[0] == 0 || h->typeFlag[0] == '0'){
				vnode->type = V_REG;
			} else if(h->typeFlag[0] == '5'){
				vnode->type = V_DIR;
			} else {
				// Links, named pipes and char/blk devices not supported.
				vnode->type = V_BAD;
			}
			vnode->size = initrd_tar_conv_number(h->size, 11);

			timespec64_t lastModifiedTime;
			lastModifiedTime.nsec = 0;
			lastModifiedTime.sec = initrd_tar_conv_number(h->mTime, 11);

			if(!time_get64().sec){
				//There may be no RTC driver loaded yet, so just set the last accessed time to the last modified time.
				vnode->lastAccessedTime = lastModifiedTime;
			} else {
				vnode->lastAccessedTime = time_get64();
			}
			vnode->lastModifiedTime = lastModifiedTime;
			vnode->lastChangedTime = lastModifiedTime;

            //the file is already in memory
            fsData->startOffset = (size_t)((uint64_t)h + ALIGN_UP(sizeof(initrd_tar_header_t), 512));
			
			vnode->fsData = fsData;

            return vnode;
        }

        //go to the next entry
        size = (size_t)initrd_tar_conv_number(h->size, 11);

        haddr += ((size / 512) + 1) * 512;
        if (size % 512) haddr += 512;

        h = (initrd_tar_header_t*)haddr;
    }	

	kerrno = ENOENT;
	return NULL;
}

statfs_t initrd_statfs(vfs_t* self){
	initrd_tar_header_t* initrd = (initrd_tar_header_t*)self->context;

	statfs_t statfs;
	statfs.type = 0x54415200; // TAR + ASCII NUL
	statfs.bsize = 512;
	statfs.totalBlocks = ALIGN_UP(module_request.response->modules[0]->size, 512)/512;
	statfs.freeBlocks = statfs.availBlocks = 0;
	statfs.files = 0; //Not like anyone is ever going to need this value anyway...
	statfs.freeFiles = 0;
	statfs.fid = self->fid;
	statfs.maxNameLength = 100;
	statfs.mountFlags = self->mountFlags;	

	return statfs;
}

//initialise initrd
void initrd_init(){
    vfs_t* initrdFS = kmalloc(sizeof(vfs_t));
    memset(initrdFS, 0, sizeof(vfs_t));

    memcpy(initrdFS->name, "init", 5);
    memcpy(initrdFS->type, FILESYS_TYPE_INIT_USTAR, strlen(FILESYS_TYPE_INIT_USTAR)+1);
    initrdFS->fsops = &initrdVFSOps;

	initrdFS->rootVnode = vnode_new(initrdFS, &initrdVnodeOps);

	initrdFS->rootVnode->flags = 0;
	initrdFS->rootVnode->type = V_DIR;
	initrdFS->rootVnode->size = 0;
	initrdFS->rootVnode->fsData = NULL;

    //fetch module address from limine
    initrd = (initrd_tar_header_t*)(module_request.response->modules[0]->address);
    if(initrd == NULL){
        panic(INITRD_FILENAME" not found!", "initrd");
    } else {
		klog("Mounting initrd...\n", KLOG_INFO, "initrd");
	}

    initrdFS->context = (void*)initrd;

    vfs_mount(initrdFS, "/", MS_RDONLY);
}
