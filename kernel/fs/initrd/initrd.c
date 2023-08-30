#include "initrd.h"
#include <fs/vfs/vfs.h>
#include <kstdio.h>
#include <sys/panic.h>
#include <mem/memutil/memutil.h>
#include <mem/mm/kheap.h>
#include <mem/mm/pmm.h>
#include <limine.h>

static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
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

vfs_file_t* initrd_open(char* path, uint8_t access, uint8_t fsNumber){

    initrd_tar_header_t* h = initrd;
    uint64_t haddr = (uint64_t)h;

    size_t size;

    for(int i = 0; ; i++){
        if(h->filename[0] == 0){
            klog("Couldn't find %s inside initrd!\n", KLOG_FAILED, path);
            panic("Module %s missing from initrd!", path);
        } else if (!strcmp(path, h->filename)){
            size = (size_t)initrd_tar_conv_number(h->size, 11);

            vfs_file_t* file = kmalloc(sizeof(vfs_file_t));
            file->access = VFS_FILE_ACCESS_R;
            file->address = (void*)((uint64_t)h + ALIGN_UP(sizeof(initrd_tar_header_t), 512));
            file->fsNumber = fsNumber;
            file->seek = 0;
            file->size = size;

            klog("initrd: Found Module \'%s\'.\n", KLOG_INFO, path);

            return file;
        }

        size = (size_t)initrd_tar_conv_number(h->size, 11);

        haddr += ((size / 512) + 1) * 512;
        if (size % 512) haddr += 512;

        h = (initrd_tar_header_t*)haddr;
    }
}

void initrd_close(vfs_file_t* file){
    kmfree((void*)file);
}

void initrd_read(vfs_file_t* file, size_t size, void* buf){

    if(file->seek > file->size){
        file->seek = file->size;
    }

    uint64_t readAddr = (uint64_t)file->address + file->seek;
    uint8_t* readPtr = (uint8_t*)readAddr;

    for(size_t i = 0; i < size; i++){
        ((uint8_t*)buf)[i] = readPtr[i];
    }

}

void initrd_write(vfs_file_t* file, size_t size, void* buf){
    return; //no need to write to the initrd
}

void initrd_remove(){
    pmm_free(module_request.response->modules[0]->address, ALIGN_UP(module_request.response->modules[0]->size, PAGE_SIZE)/PAGE_SIZE);
}

void initrd_init(){
    vfs_fs_t initrdFS;
    initrdFS.name = "init";
    initrdFS.open = initrd_open;
    initrdFS.close = initrd_close;
    initrdFS.read = initrd_read;
    initrdFS.write = initrd_write;

    initrd = (initrd_tar_header_t*)(module_request.response->modules[0]->address);
    if(initrd == NULL){
        panic("Initrd not found!");
    }

    initrdFS.context = (void*)initrd;

    vfs_add(initrdFS);
}
