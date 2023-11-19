/*  
*   File: initrd.c
*
*   Author: Garnek
*   
*   Description: ustar initial ramdisk operations
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "initrd.h"
#include <sys/fal/fal.h>
#include <kstdio.h>
#include <sys/panic.h>
#include <sys/bootloader.h>
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

file_t* initrd_open(filesys_t* fs, char* path, uint8_t access){

    initrd_tar_header_t* h = initrd;
    uint64_t haddr = (uint64_t)h;

    size_t size;

    //look through the initrd until the file is found
    //...or not found, in which case a kernel panic is triggered 
    for(int i = 0; ; i++){
        if(h->filename[0] == 0){
            klog("Couldn't find %s inside initrd!\n", KLOG_FAILED, path);
            panic("Module %s missing from initrd!", path);
        } else if (!strcmp(path, h->filename)){
            size = (size_t)initrd_tar_conv_number(h->size, 11);

            file_t* file = kmalloc(sizeof(file_t));
            file->access = FILE_ACCESS_R;
            //the file is already in memory
            file->address = (void*)((uint64_t)h + ALIGN_UP(sizeof(initrd_tar_header_t), 512));
            file->fs = fs;
            file->seek = 0;
            file->size = size;

            klog("initrd: Found Module \'%s\'.\n", KLOG_INFO, path);

            return file;
        }

        //go to the next entry
        size = (size_t)initrd_tar_conv_number(h->size, 11);

        haddr += ((size / 512) + 1) * 512;
        if (size % 512) haddr += 512;

        h = (initrd_tar_header_t*)haddr;
    }
}

void initrd_close(filesys_t* fs, file_t* file){
    kmfree((void*)file);
}

void initrd_read(filesys_t* fs, file_t* file, size_t size, void* buf){

    if(file->seek > file->size){
        file->seek = file->size;
    }

    uint64_t readAddr = (uint64_t)file->address + file->seek;
    uint8_t* readPtr = (uint8_t*)readAddr;

    for(size_t i = 0; i < size; i++){
        ((uint8_t*)buf)[i] = readPtr[i];
    }

}

void initrd_write(filesys_t* fs, file_t* file, size_t size, void* buf){
    return; //no need to write to the initrd
}

void initrd_rmdir(filesys_t* fs, char* path){
    return; //no need to remove directories from the initrd
}

void initrd_mkdir(filesys_t* fs, char* path){
    return; //no need to make directories inside the initrd
}

//initialise initrd
void initrd_init(){
    filesys_t initrdFS;

    filesys_set_name(&initrdFS, "init");
    initrdFS.type = FILESYS_TYPE_INIT_USTAR;
    initrdFS.open = initrd_open;
    initrdFS.close = initrd_close;
    initrdFS.read = initrd_read;
    initrdFS.write = initrd_write;
    initrdFS.mkdir = initrd_mkdir;
    initrdFS.rmdir = initrd_rmdir;

    //fetch module address from limine
    initrd = (initrd_tar_header_t*)(module_request.response->modules[0]->address);
    if(initrd == NULL){
        panic("Initrd not found!");
    }

    initrdFS.context = (void*)initrd;
    initrdFS.size = initrd_tar_conv_number(initrd->size, 11);

    filesys_mount(initrdFS);
}