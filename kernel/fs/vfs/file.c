/*  
*   File: file.c
*
*   Author: Garnek
*   
*   Description: File Abstraction
*/
// SPDX-License-Identifier: BSD-2-Clause
#include "file.h"
#include <mem/mm/kheap.h>
#include <kstdio.h>

//open file
file_t* kfopen(char* path, uint8_t access){
    char chr = path[0];
    uint8_t fsNumber = 0;

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
    filesys_t* fs = filesys_get(fsNumber);
    if(fs->open == NULL){
        goto invalidfsindex;
    }

    //open the file
    file_t* file;
    file = fs->open(fs, path, access);
    file->fs = fs;
    return file;

invalidfsindex:
    klog("Failed to open file \'%s\'! Invalid filesystem index.\n", KLOG_FAILED, path);
    return NULL;
}

//modify current seek position
void kfseek(file_t* file, size_t seekPos){
    file->seek = seekPos;
}

//close file
void kfclose(file_t* file){
    file->fs->close(file->fs, file);
}

//read from file
void kfread(file_t* file, size_t size, void* buf){
    if(file->access != FILE_ACCESS_R && file->access != FILE_ACCESS_RW){
        klog("File access violation!\n", KLOG_WARNING);
        return;
    }

    file->fs->read(file->fs, file, size, buf);
}

//write to file
void kfwrite(file_t* file, size_t size, void* buf){
    if(file->access != FILE_ACCESS_W && file->access != FILE_ACCESS_RW){
        klog("File access violation!\n", KLOG_WARNING);
        return;
    }

    file->fs->write(file->fs, file, size, buf);
}