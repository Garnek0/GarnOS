/*  
*   File: file.c
*
*   Author: Garnek
*   
*   Description: File Abstraction
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fal-internals.h"

#include <garn/fal/file.h>
#include <garn/mm.h>
#include <garn/kstdio.h>
#include <garn/kerrno.h>
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
                if((i->flags & O_DIRECTORY) && !(flags & O_DIRECTORY)){
                    kerrno = EISDIR;
                    releaseLock(&fs->lock);
                    return NULL;   
                }
                if(!(i->flags & O_DIRECTORY) && (flags & O_DIRECTORY)){
                    kerrno = ENOTDIR;
                    releaseLock(&fs->lock);
                    return NULL;
                }

                i->refCount++;
                releaseLock(&fs->lock);
                return i;
            }
        }
        if(!fs->fsOperations.open){
            kerrno = EINVAL;
            releaseLock(&fs->lock);
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
    if(prevSize == newSize) return fd;

    fd_t* newfd = (fd_t*)kmalloc(sizeof(fd_t)*newSize);
    memset(newfd, 0, sizeof(fd_t)*newSize);
    memcpy(newfd, fd, sizeof(fd_t)*prevSize);
    kmfree(fd);

    return newfd;
}

char* file_get_absolute_path(char* root, char* relative){
    kerrno = 0;

    //Check if path is absolute
    bool absolute = false;
    if(relative[0] == '/') absolute = true;
    else {
        char* relativeTmp = relative;
        while(relativeTmp[0] != 0){
            if(relativeTmp[0] == ':'){
                absolute = true;
                break;
            } else if(relativeTmp[0] == '/') break;
            relativeTmp++;
        }
    }
    
    size_t size = 0;
    size_t sptr = 0;

    char* buf = kmalloc(PATH_MAX+1);

    if(absolute && relative[0] != '/'){
        memcpy(buf, relative, strlen(relative)+1);
        goto checkpath;
    }
    else if(absolute && relative[0] == '/'){
        size = sizeof(relative)+2;
        if(size > PATH_MAX){
            kerrno = ENAMETOOLONG;
            return NULL;
        }

        memcpy(&buf[2], relative, strlen(relative)+1);
        buf[0] = '0';
        buf[1] = ':';
        goto checkpath;
    }

    size = strlen(root)+strlen(relative);
    if(size > PATH_MAX){
        kerrno = ENAMETOOLONG;
        return NULL;
    }

    memcpy(buf, root, strlen(root));
    sptr = strlen(root);
    if(root[sptr-1]!='/'){
        size++;
        if(size > PATH_MAX){
            kmfree(buf);
            kerrno = ENAMETOOLONG;
            return NULL;
        }
        buf[sptr] = '/';
        sptr++;
    }
    memcpy(&buf[sptr], relative, strlen(relative)+1);
    strncat(buf, "/", 1);

checkpath:

    int dirindex = 0;
    for(uint32_t i = 0; i < strlen(buf); i++){
        if(buf[i] == '/') dirindex++;
        if(!strncmp(&buf[i], "/../", 4)){
            if(dirindex < 2){
                memcpy(&buf[i], &buf[i+3], strlen(&buf[i+3])+1);
                continue;
            }

            for(int j = i-1; j > 0; j--){
                if(buf[j] == '/'){
                    memcpy(&buf[j], &buf[i+3], strlen(&buf[i+3])+1);
                    break;
                }
            }
        } else if(!strncmp(&buf[i], "/..", 3)){
            if(dirindex < 2){
                memcpy(&buf[i], &buf[i+3], strlen(&buf[i+3])+1);
                continue;
            }

            for(int j = i-1; j > 0; j--){
                if(buf[j] == '/'){
                    memcpy(&buf[j+1], &buf[i+3], strlen(&buf[i+3])+1);
                    break;
                }
            }
        } else if(!strncmp(&buf[i], "/.", 2)){
            memcpy(&buf[i], &buf[i+2], strlen(&buf[i+2])+1);
        } else if(!strncmp(&buf[i], "//", 2)){
            memcpy(&buf[i], &buf[i+1], strlen(&buf[i+1])+1);
        }
    }

    return buf;
}

//TODO: sys_stat() syscall
//TODO: sys_lseek() syscall

int sys_open(stack_frame_t* regs, char* path, int flags, int mode){
    process_t* currentProcess = sched_get_current_process();

    char* absPath = file_get_absolute_path(currentProcess->cwd, path);

findfd:

    for(size_t i = 0; i < currentProcess->fdMax; i++){
        if(!currentProcess->fdTable[i].file){
            currentProcess->fdTable[i].file = file_open(absPath, flags, mode);
            if(currentProcess->fdTable[i].file == NULL) return -kerrno;
            if(flags & O_APPEND) currentProcess->fdTable[i].offset = currentProcess->fdTable[i].file->size;
            else currentProcess->fdTable[i].offset = 0;
            currentProcess->fdTable[i].offset = 0;
            currentProcess->fdTable[i].flags = flags;
            kmfree(absPath);
            return i;
        }
    }

    //no available fd found, try to realloc fd table
    if((currentProcess->fdMax+1) >= PROCESS_MAX_FD) return -EMFILE;
    file_realloc_fd_table(currentProcess->fdTable, currentProcess->fdMax+1, (currentProcess->fdMax+1)*2);
    currentProcess->fdMax = (currentProcess->fdMax+1)*2-1;
    goto findfd;
}

ssize_t sys_read(stack_frame_t* regs, int fd, void* buf, size_t count){
    process_t* currentProcess = sched_get_current_process();

    if((size_t)fd < 0 || (size_t)fd > currentProcess->fdMax || !currentProcess->fdTable[fd].file) return (ssize_t)-EBADF;

    fd_t* currentfd = &currentProcess->fdTable[fd];

    if(!(currentfd->flags & O_RDONLY) && !(currentfd->flags & O_RDWR)) return (ssize_t)-EACCES;

    size_t res = file_read(currentProcess->fdTable[fd].file, count, buf, currentProcess->fdTable[fd].offset);
    if(res > 0) currentProcess->fdTable[fd].offset += res;

    return res;
}

ssize_t sys_write(stack_frame_t* regs, int fd, void* buf, size_t count){
    process_t* currentProcess = sched_get_current_process();

    if((size_t)fd < 0 || (size_t)fd > currentProcess->fdMax || !currentProcess->fdTable[fd].file) return (ssize_t)-EBADF;

    fd_t* currentfd = &currentProcess->fdTable[fd];

    if(!(currentfd->flags & O_WRONLY) && !(currentfd->flags & O_RDWR)) return (ssize_t)-EACCES;

    size_t res = file_write(currentProcess->fdTable[fd].file, count, buf, currentProcess->fdTable[fd].offset);
    if(res > 0) currentProcess->fdTable[fd].offset += res;

    return res;
}

int sys_close(stack_frame_t* regs, int fd){
    process_t* currentProcess = sched_get_current_process();

    if((size_t)fd < 0 || (size_t)fd > currentProcess->fdMax || !currentProcess->fdTable[fd].file) return -EBADF;

    fd_t* currentfd = &currentProcess->fdTable[fd];

    currentfd->file = NULL;

    return 0;
}

uint64_t sys_getcwd(stack_frame_t* regs, const char* buf, size_t size){
    process_t* currentProcess = sched_get_current_process();
    if(size < strlen(currentProcess->cwd)+1) return -ERANGE;

    memcpy((void*)buf, (void*)currentProcess->cwd, strlen(currentProcess->cwd)+1);
    return (uint64_t)buf;
}

int sys_chdir(stack_frame_t* regs, const char* path){
    if(strlen(path) > PATH_MAX) return -ENAMETOOLONG;

    process_t* currentProcess = sched_get_current_process();

    char* str = file_get_absolute_path(currentProcess->cwd, (char*)path);
    if(str == NULL) return -kerrno;
    if(str[strlen(str)-1]!='/'){
        char* strOk = kmalloc(strlen(str)+2);
        memcpy(strOk, str, strlen(str)+1);
        strncat(strOk, "/", 1);
        strOk[strlen(strOk)] = 0;
        kmfree(str);
        str = strOk;
    }

    file_t* file = file_open(str, O_DIRECTORY | O_RDONLY, 0);
    if(!file){
        kmfree(str);
        return -kerrno;
    }

    kmfree(currentProcess->cwd);
    currentProcess->cwd = str;
    return 0;
}

ssize_t sys_getdents(stack_frame_t* regs, int fd, void* dirp, size_t count){
    if(!dirp) return -EFAULT;

    process_t* currentProcess = sched_get_current_process();

    if(!currentProcess->fdTable[fd].file) return -EBADF;
    if(!(currentProcess->fdTable[fd].file->flags & O_DIRECTORY)) return -ENOTDIR;

    ssize_t bytesRead = file_read(currentProcess->fdTable[fd].file, count, dirp, currentProcess->fdTable[fd].offset);

    currentProcess->fdTable[fd].offset += bytesRead;

    return bytesRead;
}