/*  
*   File: vfs.c
*
*   Author: Garnek
*   
*   Description: GarnOS virtual filesystem implementation
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "vfs.h"
#include <mem/memutil/memutil.h>
#include <kstdio.h>

vfs_fs_t filesystems[VFS_MAX_FILESYSTEMS];
bool availFSNumbers[VFS_MAX_FILESYSTEMS] = {false}; //TODO: make this a bitmap
uint16_t vfsNextAvailFSNumber;

//The GarnOS virtual filesystem is a simple indexed vfs model.
//It assigns numbers to drives (Just like how (Free)DOS/Windows assigns letters to drives)
//ranging from 0 to 255; 0 is reserved for the Initrd.

void vfs_add(vfs_fs_t fs){
    fs.flags |= (VFS_FS_AVAIL);
    fs.mountNumber = vfsNextAvailFSNumber;

    filesystems[vfsNextAvailFSNumber] = fs;
    availFSNumbers[vfsNextAvailFSNumber] = true;
    for(uint16_t i = 0; i < VFS_MAX_FILESYSTEMS; i++){
        if(!availFSNumbers[i]){
            vfsNextAvailFSNumber = i;
            break;
        }
    }

    klog("Added New Filesystem %d: (%s) Successfully.\n", KLOG_OK, fs.mountNumber, fs.name);
}

void vfs_remove(uint8_t index){

    if(!(filesystems[index].flags & VFS_FS_AVAIL)){
        klog("Attempt to remove nonexistent filesystem :%d!\n", KLOG_WARNING, index);
        return;
    }
    memset(&filesystems[index], 0, sizeof(vfs_fs_t));

    availFSNumbers[index] = false;

    klog("Removed Filesystem %d: Successfully.\n", KLOG_OK, index);
}

vfs_file_t* vfs_open(char* path, uint8_t access){
    char chr = path[0];
    uint8_t fsNumber = 0;

    //get drive number
    if(chr > '9' || chr < '0'){
        goto invalidfsindex;
    }

    while(chr <= '9' && chr >= '0'){
        fsNumber *= 10;
        fsNumber += (uint8_t)(chr - '0');
        path++;
        chr = path[0];
    }
    //drive number should be followed by ":/"
    if(chr != ':'){
        goto invalidfsindex;
    }
    path++;
    chr = path[0];
    if(chr != '/'){
        goto invalidfsindex;
    }
    path++;

    //make sure the filesystem exists
    if(filesystems[fsNumber].open == NULL){
        goto invalidfsindex;
    }

    //open the file
    vfs_file_t* file;
    file = filesystems[fsNumber].open(path, access, fsNumber);
    file->fsNumber = fsNumber;
    return file;

invalidfsindex:
    klog("Failed to open file \'%s\'! Invalid filesystem index.\n", KLOG_FAILED, path);
    return NULL;
}

//modify current seek position
void vfs_seek(vfs_file_t* file, size_t seekPos){
    file->seek = seekPos;
}

//close file
void vfs_close(vfs_file_t* file){
    filesystems[file->fsNumber].close(file);
}

//read from file
void vfs_read(vfs_file_t* file, size_t size, void* buf){
    if(file->access != VFS_FILE_ACCESS_R && file->access != VFS_FILE_ACCESS_RW){
        klog("File access violation!\n", KLOG_WARNING);
        return;
    }

    filesystems[file->fsNumber].read(file, size, buf);
}

//write to file
void vfs_write(vfs_file_t* file, size_t size, void* buf){

    if(file->access != VFS_FILE_ACCESS_W && file->access != VFS_FILE_ACCESS_RW){
        klog("File access violation!\n", KLOG_WARNING);
        return;
    }

    filesystems[file->fsNumber].write(file, size, buf);
}