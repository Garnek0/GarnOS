/*  
*   File: vnode.c
*
*   Author: Garnek
*   
*   Description: Vnode Implementation (File abstraction)
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fal-internals.h"

#include <garn/fal/vnode.h>
#include <garn/mm.h>
#include <garn/kstdio.h>
#include <garn/kerrno.h>
#include <process/sched/sched.h>

/*TODO:
- [x] Add proper flag-checking to all of the vnode functions (open, read, write etc)
- [x] readdir
- [x] Add some way to get filesystem drivers from modules
- [ ] Add FAT(12/16/)32 support (again)
- [ ] Add pts or something for stdin/stdout/stderr
- [ ] Add procfs
*/

vnode_t* vnodeListHead;
vnode_t* vnodeListTail;

void vnode_list_add(vnode_t* vnode){
    if(vnodeListHead == NULL){
        vnodeListHead = vnodeListTail = vnode;
        vnodeListHead->next = NULL;
        vnodeListHead->prev = NULL;
    } else {
        vnodeListTail->next = vnode;
        vnode->prev = vnodeListTail;
        vnodeListTail = vnode;
        vnodeListTail->next = NULL;
    }
}

void vnode_list_remove(vnode_t* vnode){
    if(vnodeListTail == vnode) vnodeListTail = vnode->prev;
    if(vnodeListHead == vnode) vnodeListHead = vnode->next;

    if(vnode->next) vnode->next->prev = vnode->prev;
    if(vnode->prev) vnode->prev->next = vnode->next;
}

void vnode_list_reset(){
	vnodeListHead = vnodeListTail = NULL;
}

vnode_t* vnode_list_search_by_filename(vfs_t* vfs, const char* filename){
	for(vnode_t* i = vnodeListHead; i; i = i->next){
		if(i->vfs->fid == vfs->fid && !strcmp(i->filename, filename)){
			return i;
		}
	}

	return NULL;
}

vnode_t* vnode_new(vfs_t* vfs, vnode_operations_t* vnodeops){
	vnode_t* vnode = kmalloc(sizeof(vnode_t));
	memset(vnode, 0, sizeof(vnode_t));

	vnode->vfs = vfs;
	vnode->vnodeops = vnodeops;

	return vnode;
}

//lookup path name
vnode_t* vnode_lookup(const char* path){
	kerrno = 0;

	if(!path || path[0] != '/'){
		kerrno = EINVAL;
		return NULL;
	}

	int stridx = 1;
	int dirnameidx = 0;
	char dirname[256];

	vfs_t* vfs = vfs_get_root();

	vnode_t* currentVnode = vfs->rootVnode;

	while(path[stridx] != 0){
		dirnameidx = 0;
		while(path[stridx] != '/'){
			dirname[dirnameidx] = path[stridx];
			dirnameidx++;
			stridx++;

			if(path[stridx] == 0) break;
		}
		if(path[stridx] == '/'){
			dirname[dirnameidx] = '/';
			dirnameidx++;
			stridx++;
		}
		dirname[dirnameidx] = 0;

		vnode_t* prevVnode = currentVnode;

		lock(prevVnode->lock, {
			if(!currentVnode->vnodeops || !currentVnode->vnodeops->vn_lookup){
				releaseLock(&currentVnode->lock);
				kerrno = EINVAL;
				return NULL;
			}

			currentVnode = currentVnode->vnodeops->vn_lookup(currentVnode, dirname);
		});	

		if((prevVnode != vfs->rootVnode) && prevVnode->vnodeops->vn_inactive) prevVnode->vnodeops->vn_inactive(prevVnode);

		if(!currentVnode){
			//kerrno should be set
			return NULL;
		}

		if(currentVnode->vfsMountedHere){
			vfs = currentVnode->vfsMountedHere;
			currentVnode = vfs->rootVnode;
			//klog("DEBUG: Found new VFS in path %s\n", KLOG_INFO, "FAL", dirname);
		}
	}

	return currentVnode;
}

//open vnode
vnode_t* vnode_open(const char* path, int flags, int mode){
	kerrno = 0;

	vnode_t* vnode = vnode_lookup(path);
	if(!vnode){
		//kerrno should be set
		return NULL;
	}

	lock(vnode->lock, {
		if((flags & O_DIRECTORY) && (vnode->type != V_DIR)){
			kerrno = ENOTDIR;
			releaseLock(&vnode->lock);
			return NULL;
		} else if(!(flags & O_DIRECTORY) && (vnode->type == V_DIR)){
			kerrno = EISDIR;
			releaseLock(&vnode->lock);
			return NULL;
		}

		if(flags & O_CREAT){
			klog("O_CREAT Not supported! Flag ignored (this may cause issues!).\n", KLOG_WARNING, "FAL");
		}

		vnode->refCount++;
		if(!vnode_list_search_by_filename(vnode->vfs, path)){
			vnode_list_add(vnode);
			vnode->flags = flags;
			vnode->mode = mode;
		}
	});
	
	return vnode;
}

//close vnode
int vnode_close(vnode_t* vnode){
    kerrno = 0;

	int status = 0;

	lock(vnode->lock, {
		vnode->refCount--;
		if(vnode->refCount == 0){
			vnode_list_remove(vnode);
			if(vnode->vnodeops->vn_inactive) status = vnode->vnodeops->vn_inactive(vnode);
			else kmfree(vnode);
		}
	});
	return status;
}

//read from vnode
ssize_t vnode_read(vnode_t* vnode, size_t size, void* buf, size_t offset){
	kerrno = 0;

	ssize_t bytesRead;

	lock(vnode->lock, {
		if(!vnode->vnodeops || !vnode->vnodeops->vn_read){
			kerrno = EINVAL;
			releaseLock(&vnode->lock);
			return -EINVAL;
		}

		//vnode_readdir should be used instead for directories
		if(vnode->type == V_DIR){
			kerrno = EINVAL;
			releaseLock(&vnode->lock);
			return -EINVAL;
		}

		bytesRead = vnode->vnodeops->vn_read(vnode, size, buf, offset);	
	});

	return bytesRead;
}

//write to vnode
ssize_t vnode_write(vnode_t* vnode, size_t size, void* buf, size_t offset){
	kerrno = 0;

	ssize_t bytesWritten;

	lock(vnode->lock, {
		if(vnode->type == V_DIR){
			kerrno = EINVAL;
			releaseLock(&vnode->lock);
			return -EINVAL;
		}

		if(!vnode->vnodeops || !vnode->vnodeops->vn_write){
			kerrno = EINVAL;
			releaseLock(&vnode->lock);
			return -EINVAL;
		}

		bytesWritten = vnode->vnodeops->vn_write(vnode, size, buf, offset);	
	});

	return bytesWritten;
}

ssize_t vnode_readdir(vnode_t* vnode, size_t count, void* buf, size_t offset){
	kerrno = 0;

	ssize_t bytesReturned;

	lock(vnode->lock, {
		if(vnode->type != V_DIR){
			kerrno = EINVAL;
			releaseLock(&vnode->lock);
			return -EINVAL;
		}

		if(!vnode->vnodeops || !vnode->vnodeops->vn_readdir){
			kerrno = EINVAL;
			releaseLock(&vnode->lock);
			return -EINVAL;
		}

		bytesReturned = vnode->vnodeops->vn_readdir(vnode, count, buf, offset);
	});

	return bytesReturned;
}

fd_t* vnode_alloc_fd_table(size_t size){
    fd_t* fd = (fd_t*)kmalloc(sizeof(fd_t)*size); 
    memset(fd, 0, sizeof(fd_t)*size);

    return fd;
}

fd_t* vnode_realloc_fd_table(fd_t* fd, size_t prevSize, size_t newSize){
    if(prevSize == newSize) return fd;

    fd_t* newfd = (fd_t*)kmalloc(sizeof(fd_t)*newSize);
    memset(newfd, 0, sizeof(fd_t)*newSize);
    memcpy(newfd, fd, sizeof(fd_t)*prevSize);
    kmfree(fd);

    return newfd;
}

char* vnode_get_absolute_path(char* root, char* relative){
    kerrno = 0;

	char* buf = kmalloc(PATH_MAX+1);
	size_t size = 0;
    size_t sptr = 0;

	size = strlen(root)+strlen(relative);
    if(size > PATH_MAX){
        kerrno = ENAMETOOLONG;
        return NULL;
    }

    //Check if path is absolute
    if(relative[0] == '/'){
		memcpy(buf, relative, strlen(relative)+1);
        goto checkpath;
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

    char* absPath = vnode_get_absolute_path(currentProcess->cwd, path);

findfd:

    for(size_t i = 0; i < currentProcess->fdMax; i++){
        if(!currentProcess->fdTable[i].file){
            currentProcess->fdTable[i].file = vnode_open(absPath, flags, mode);
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
    vnode_realloc_fd_table(currentProcess->fdTable, currentProcess->fdMax+1, (currentProcess->fdMax+1)*2);
    currentProcess->fdMax = (currentProcess->fdMax+1)*2-1;
    goto findfd;
}

ssize_t sys_read(stack_frame_t* regs, int fd, void* buf, size_t count){
    process_t* currentProcess = sched_get_current_process();

    if((size_t)fd < 0 || (size_t)fd > currentProcess->fdMax || !currentProcess->fdTable[fd].file) return (ssize_t)-EBADF;

    fd_t* currentfd = &currentProcess->fdTable[fd];

    if((currentfd->flags & O_WRONLY) && !(currentfd->flags & O_RDWR)) return (ssize_t)-EACCES;

    size_t res = vnode_read(currentProcess->fdTable[fd].file, count, buf, currentProcess->fdTable[fd].offset);
    if(res > 0) currentProcess->fdTable[fd].offset += res;

    return res;
}

ssize_t sys_write(stack_frame_t* regs, int fd, void* buf, size_t count){
	process_t* currentProcess = sched_get_current_process();

    if((size_t)fd < 0 || (size_t)fd > currentProcess->fdMax || !currentProcess->fdTable[fd].file) return (ssize_t)-EBADF;

    fd_t* currentfd = &currentProcess->fdTable[fd];

    if(!(currentfd->flags & O_WRONLY) && !(currentfd->flags & O_RDWR)) return (ssize_t)-EACCES;

    size_t res = vnode_write(currentProcess->fdTable[fd].file, count, buf, currentProcess->fdTable[fd].offset);
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

    char* str = vnode_get_absolute_path(currentProcess->cwd, (char*)path);
    if(str == NULL) return -kerrno;
    if(str[strlen(str)-1]!='/'){
        char* strOk = kmalloc(strlen(str)+2);
        memcpy(strOk, str, strlen(str)+1);
        strncat(strOk, "/", 1);
        strOk[strlen(strOk)] = 0;
        kmfree(str);
        str = strOk;
    }

    vnode_t* file = vnode_open(str, O_DIRECTORY | O_RDONLY, 0);
    if(!file){
        kmfree(str);
        return -kerrno;
    }

    kmfree(currentProcess->cwd);
    currentProcess->cwd = str;
    return 0;
}

long sys_getdents(stack_frame_t* regs, int fd, void* dirp, unsigned int count){
	if(!dirp) return -EFAULT;

    process_t* currentProcess = sched_get_current_process();

    if(!currentProcess->fdTable[fd].file) return -EBADF;
    if(!(currentProcess->fdTable[fd].file->flags & O_DIRECTORY)) return -ENOTDIR;

	ssize_t bytesRead = vnode_readdir(currentProcess->fdTable[fd].file, count, dirp, currentProcess->fdTable[fd].offset);

    currentProcess->fdTable[fd].offset += bytesRead;

    return bytesRead;
}
