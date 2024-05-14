/*  
*   File: initrd.c
*
*   Author: Garnek
*   
*   Description: ustar initial ramdisk operations
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "initrd.h"
#include <garn/fal/file.h>
#include <garn/fal/filesys.h>
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

file_t* initrd_open(filesys_t* fs, char* path, int flags, int mode){

    initrd_tar_header_t* h = initrd;
    uint64_t haddr = (uint64_t)h;

    size_t size;

    //look through the initrd until the file is found
    //...or not found, in which case a kernel panic is triggered 
    for(int i = 0; ; i++){
        if(h->filename[0] == 0){
            klog("Couldn't find %s\n", KLOG_FAILED, "initrd", path);
            kerrno = ENOENT;
            return NULL;
        } else if (!strcmp(path, h->filename)){
            size = (size_t)initrd_tar_conv_number(h->size, 11);

            file_t* file = kmalloc(sizeof(file_t));
            memset(file, 0, sizeof(file_t));
            initrd_file_fs_data_t* fsData = kmalloc(sizeof(initrd_file_fs_data_t));
            file->filename = kmalloc(strlen(path)+1);
            memcpy(file->filename, path, strlen(path)+1);
            file->mode = mode;
            file->flags = flags;
            file->fsData = (void*)fsData;
            file->fs = fs;
            file->size = size;

            //the file is already in memory
            fsData->startOffset = (size_t)((uint64_t)h + ALIGN_UP(sizeof(initrd_tar_header_t), 512));

            return file;
        }

        //go to the next entry
        size = (size_t)initrd_tar_conv_number(h->size, 11);

        haddr += ((size / 512) + 1) * 512;
        if (size % 512) haddr += 512;

        h = (initrd_tar_header_t*)haddr;
    }
}

int initrd_close(filesys_t* fs, file_t* file){
    kmfree((void*)file->fsData);
    kmfree((void*)file);
    return 0;
}

ssize_t initrd_read(filesys_t* fs, file_t* file, size_t size, void* buf, size_t offset){
    initrd_file_fs_data_t* fsData = (initrd_file_fs_data_t*)file->fsData;

    for(size_t i = 0; i < size; i++){
        if(offset >= file->size) return i;
        ((uint8_t*)buf)[i] = ((uint8_t*)fsData->startOffset)[offset];
        offset++;
    }
    return size;
}

ssize_t initrd_write(filesys_t* fs, file_t* file, size_t size, void* buf, size_t offset){
    kerrno = EINVAL;
    return -EINVAL; //no need to write to the initrd
}

int initrd_rmdir(filesys_t* fs, char* path){
    kerrno = EINVAL;
    return -1; //no need to remove directories from the initrd
}

int initrd_mkdir(filesys_t* fs, char* path){
    kerrno = EINVAL;
    return -1; //no need to make directories inside the initrd
}

//initialise initrd
void initrd_init(){
    filesys_t initrdFS;
    memset(&initrdFS, 0, sizeof(filesys_t));

    memcpy(initrdFS.name, "init", 5);
    memcpy(initrdFS.type, FILESYS_TYPE_INIT_USTAR, strlen(FILESYS_TYPE_INIT_USTAR)+1);
    initrdFS.fsOperations.open = initrd_open;
    initrdFS.fsOperations.close = initrd_close;
    initrdFS.fsOperations.read = initrd_read;
    initrdFS.fsOperations.write = initrd_write;
    initrdFS.fsOperations.mkdir = initrd_mkdir;
    initrdFS.fsOperations.rmdir = initrd_rmdir;

    //fetch module address from limine
    initrd = (initrd_tar_header_t*)(module_request.response->modules[0]->address);
    if(initrd == NULL){
        panic(INITRD_FILENAME" not found!", "initrd");
    }

    initrdFS.context = (void*)initrd;
    initrdFS.size = initrd_tar_conv_number(initrd->size, 11);

    filesys_mount(initrdFS);
}