/*  
*   File: file.c
*
*   Author: Garnek
*   
*   Description: File Abstraction
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "file.h"
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>
#include <kstdio.h>
#include <kerrno.h>
#include <process/sched/sched.h>

file_t* openFiles;
file_t* openFileLast;

void file_list_add(file_t* file){
    if(openFiles == NULL){
        openFileLast = openFiles = file;
        openFiles->next = NULL;
        openFiles->prev = NULL;
    } else {
        openFileLast->next = file;
        file->prev = openFileLast;
        openFileLast = file;
        openFileLast->next = NULL;
    }
}

void file_list_remove(file_t* file){
    if(openFileLast == file) openFileLast = file->prev;
    if(openFiles == file) openFiles = file->next;

    if(file->next) file->next->prev = file->prev;
    if(file->prev) file->prev->next = file->next;
}

//open file
file_t* file_open(char* path, int flags, int mode){
    kerrno = 0;

    uint8_t fsNumber = 0;

    if(path[0] > '9' || path[0] < '0'){
        goto invalidfsindex;
    }

    while(path[0] <= '9' && path[0] >= '0'){
        fsNumber *= 10;
        fsNumber += (uint8_t)(path[0] - '0');
        path++;
    }
    //drive number should be followed by ":/"
    if(path[0] != ':'){
        goto invalidfsindex;
    }
    path++;
    if(path[0] != '/'){
        goto invalidfsindex;
    }
    path++;
    //make sure the filesystem exists
    filesys_t* fs = filesys_get(fsNumber);
    lock(fs->lock, {
        for(file_t* i = openFiles; i != NULL; i = i->next){
            if(i->fs == fs && !strcmp(i->filename, path)){
                if(((i->flags & O_DIRECTORY) && !(flags & O_DIRECTORY) ||
                !(i->flags & O_DIRECTORY) && (flags & O_DIRECTORY))){
                    kerrno = EISDIR;
                    return NULL;
                }

                i->refCount++;
                releaseLock(&fs->lock);
                return i;
            }
        }

        if(!fs->fsOperations.open){
            kerrno = EINVAL;
            return NULL;
        }
        //open the file
        file_t* file;
        file = fs->fsOperations.open(fs, path, flags, mode);
        if(file == NULL){
            //kerrno should have already been set by the fs driver
            releaseLock(&fs->lock);
            return NULL;
        }
        file->fs = fs;
        file->refCount = 1;

        file_list_add(file);

        releaseLock(&fs->lock);
        return file;
    });

invalidfsindex:
    kerrno = ENOENT;
    return NULL;
}

//close file
int file_close(file_t* file){
    lock(file->lock, {
        file->refCount--;
        if(file->refCount == 0){
            if(!file->fs->fsOperations.close){
                releaseLock(&file->lock);
                return -EINVAL;
            }

            file_list_remove(file);

            int res = file->fs->fsOperations.close(file->fs, file);
            return res;
        }
    });
    return 0;
}

//read from file
ssize_t file_read(file_t* file, size_t size, void* buf, size_t offset){
    lock(file->lock, {
        if(!file->fs->fsOperations.read){
            releaseLock(&file->lock);
            return -EINVAL;
        }
        int res = file->fs->fsOperations.read(file->fs, file, size, buf, offset);
        releaseLock(&file->lock);
        return res;
    });
}

//write to file
ssize_t file_write(file_t* file, size_t size, void* buf, size_t offset){
    lock(file->lock, {
        if(!file->fs->fsOperations.write){
            releaseLock(&file->lock);
            return -EINVAL;
        }
        int res = file->fs->fsOperations.write(file->fs, file, size, buf, offset);
        releaseLock(&file->lock);
        return res;
    });
}

fd_t* file_alloc_fd_table(size_t size){
    fd_t* fd = (fd_t*)kmalloc(sizeof(fd_t)*size); 
    memset(fd, 0, sizeof(fd_t)*size);

    return fd;
}

fd_t* file_realloc_fd_table(fd_t* fd, size_t prevSize, size_t newSize){
    if(prevSize >= newSize) return;

    fd_t* newfd = (fd_t*)kmalloc(sizeof(fd_t)*newSize);
    memset(newfd, 0, sizeof(fd_t)*newSize);
    memcpy(newfd, fd, sizeof(fd_t)*prevSize);

    return newfd;
}

//TODO: sys_stat() syscalls;

int sys_open(char* pathname, int flags, int mode){
    //TODO: finish this (flags, mode)

    process_t* currentProcess = sched_get_current_process();

    char absPath[PATH_MAX+1];

findfd:

    for(size_t i = 0; i < currentProcess->fdMax; i++){
        if(!currentProcess->fdTable[i].file){
            memcpy(&absPath[0], currentProcess->cwd, strlen(currentProcess->cwd));
            memcpy(&absPath[strlen(currentProcess->cwd)], pathname, strlen(pathname)+1);
            currentProcess->fdTable[i].file = file_open(pathname, flags, mode);
            if(currentProcess->fdTable[i].file == NULL) return -kerrno;
            if(flags & O_APPEND) currentProcess->fdTable[i].offset = currentProcess->fdTable[i].file->size;
            else currentProcess->fdTable[i].offset = 0;
            currentProcess->fdTable[i].offset = 0;
            currentProcess->fdTable[i].flags = flags;
            return i;
        }
    }

    //no available fd found, try to realloc fd table
    if((currentProcess->fdMax+1) >= PROCESS_MAX_FD) return -EMFILE;
    file_realloc_fd_table(currentProcess->fdTable, currentProcess->fdMax+1, (currentProcess->fdMax+1)*2);
    currentProcess->fdMax = (currentProcess->fdMax+1)*2-1;
    goto findfd;
}

ssize_t sys_read(int fd, void* buf, size_t count){
    process_t* currentProcess = sched_get_current_process();

    if(fd < 0 || fd > currentProcess->fdMax || !currentProcess->fdTable[fd].file) return -EBADF;

    fd_t* currentfd = &currentProcess->fdTable[fd];

    if(!(currentfd->flags & O_RDONLY) && !(currentfd->flags & O_RDWR)) return -EACCES;

    size_t res = file_read(currentProcess->fdTable[fd].file, count, buf, currentProcess->fdTable[fd].offset);
    if(res >= 0) currentProcess->fdTable[fd].offset += res;

    return res;
}

ssize_t sys_write(int fd, void* buf, size_t count){
    process_t* currentProcess = sched_get_current_process();

    if(fd < 0 || fd > currentProcess->fdMax || !currentProcess->fdTable[fd].file) return -EBADF;

    fd_t* currentfd = &currentProcess->fdTable[fd];

    if(!(currentfd->flags & O_WRONLY) && !(currentfd->flags & O_RDWR)) return -EACCES;

    size_t res = file_write(currentProcess->fdTable[fd].file, count, buf, currentProcess->fdTable[fd].offset);
    if(res >= 0) currentProcess->fdTable[fd].offset += res;

    return res;
}

int sys_close(int fd){
    process_t* currentProcess = sched_get_current_process();

    if(fd < 0 || fd > currentProcess->fdMax || !currentProcess->fdTable[fd].file) return -EBADF;

    fd_t* currentfd = &currentProcess->fdTable[fd];

    currentfd->file = NULL;

    return 0;
}

uint64_t sys_getcwd(const char* buf, size_t size){
    process_t* currentProcess = sched_get_current_process();
    if(size < strlen(currentProcess->cwd)+1) return -ERANGE;

    memcpy(buf, currentProcess->cwd, strlen(currentProcess->cwd)+1);
    return (uint64_t)buf;
}

int sys_chdir(const char* path){
    if(strlen(path) > PATH_MAX) return -ENAMETOOLONG;

    process_t* currentProcess = sched_get_current_process();

    char buf[PATH_MAX+1];

    //Check if path is absolute
    bool absolute = false;
    char* pathTmp = path;
    while(pathTmp[0] != 0){
        if(pathTmp[0] == ':'){
            absolute = true;
            break;
        } else if(pathTmp[0] == '/') break;
        pathTmp++;
    } 
    if(absolute){
        file_t* fdir = file_open(path, (O_RDONLY | O_DIRECTORY), 0);
        if(!fdir) return -kerrno;

        kmfree(currentProcess->cwd);
        currentProcess->cwd = strdup(path);
    } else {
        if(strlen(currentProcess->cwd) + strlen(path) > PATH_MAX) return -ENAMETOOLONG;
        memcpy(&buf[0], currentProcess->cwd, strlen(currentProcess->cwd));
        memcpy(&buf[strlen(currentProcess->cwd)], path, strlen(path)+1);

        file_t* fdir = file_open(buf, (O_RDONLY | O_DIRECTORY), 0);
        if(!fdir) return -kerrno;

        kmfree(currentProcess->cwd);
        currentProcess->cwd = strdup(buf);
    }
    return 0;
}