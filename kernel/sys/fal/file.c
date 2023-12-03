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
#include <kerrno.h>
#include <cpu/smp/spinlock.h>

spinlock_t fileLock;

//open file
file_t* kfopen(char* path, uint8_t access){
    kerrno = 0;

    char chr = path[0];
    uint8_t fsNumber = 0;

    lock(fileLock, {
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
        if(file == NULL){
            //kerrno should have already been set by the fs driver
            releaseLock(&fileLock);
            return NULL;
        }
        file->fs = fs;
        releaseLock(&fileLock);
        return file;

invalidfsindex:
        kerrno = ENOENT;
        releaseLock(&fileLock);
        return NULL;
    });
}

//modify current seek position
int kfseek(file_t* file, int seekPos, int whence){
    kerrno = 0;

    lock(fileLock, {
        if(whence == FILE_SEEK_SET){
            file->seek = seekPos;
        } else if(whence == FILE_SEEK_CUR){
            file->seek += seekPos;
        } else if(whence == FILE_SEEK_END){
            file->seek = file->size + seekPos;
        } else {
            releaseLock(&fileLock);
            return -1;
        }
    });
    return 0;
}

//close file
int kfclose(file_t* file){
    kerrno = 0;

    lock(fileLock, {
        int res = file->fs->close(file->fs, file);
        releaseLock(&fileLock);
        return res;
    });
}

//read from file
int kfread(file_t* file, size_t size, void* buf){
    kerrno = 0;

    lock(fileLock, {
        if(file->access != FILE_ACCESS_R && file->access != FILE_ACCESS_RW){
            kerrno = EACCES;
            releaseLock(&fileLock);
            return -1;
        }

        int res = file->fs->read(file->fs, file, size, buf);
        releaseLock(&fileLock);
        return res;
    });
}

//write to file
int kfwrite(file_t* file, size_t size, void* buf){
    kerrno = 0;

    lock(fileLock, {
        if(file->access != FILE_ACCESS_W && file->access != FILE_ACCESS_RW){
            kerrno = EACCES;
            releaseLock(&fileLock);
            return -1;
        }

        int res = file->fs->write(file->fs, file, size, buf);
        releaseLock(&fileLock);
        return res;
    });
}