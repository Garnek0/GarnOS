/*  
*   File: fat_open.c
*
*   Author: Garnek
*   
*   Description: FAT Driver open routine
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fat.h"
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>
#include <kerrno.h>
#include <kstdio.h>

file_t* fat_open(filesys_t* self, char* path, int flags, int mode){
    kerrno = 0;

    fat_context_t* context = (fat_context_t*)self->context;
    fat_directory_t* currentDir = NULL;
    fat_lfn_t* currentLFN = NULL;
    bcache_buf_t* buf = NULL;
    bool hasEndSlash = false;
    char* pathTmp = kmalloc(strlen(path)+1);
    memcpy(pathTmp, path, strlen(path)+1);

    if(!strcmp(self->type, FILESYS_TYPE_FAT12)){
        //TODO: This
    } else if(!strcmp(self->type, FILESYS_TYPE_FAT16)){
        //TODO: This
    } else {
        fat32_ebpb_t* fat32ebpb = (fat32_ebpb_t*)context->ebpb;
        
        char dir[256]; //holds the filename of the current object
        bool isTargetObject = false;
        size_t strptr = 0; //used for getting filenames from the path string

        const size_t partitionOffset = self->drive->partitions[self->partition].startLBA; //must be added to read from the correct partition
        size_t currentSector = partitionOffset + context->firstRootDirSector;
        uint32_t currentCluster = fat32ebpb->rootDirCluster;

        bool found;
        bool isLFNDirectory;
        bool foundByLFN;

        //This is a path to the root dir
        if(strlen(path) == 0){
            if(!(flags &O_DIRECTORY) || ((flags & O_WRONLY) || ((flags & O_RDWR)))){
                kerrno = ENOENT;
                goto fail;
            }
            file_t* file = kmalloc(sizeof(file_t));
            memset(file, 0, sizeof(file_t));
            fat_file_fs_data_t* fsData = kmalloc(sizeof(fat_file_fs_data_t));
            {
                int e = 0;
                int clus = fat32ebpb->rootDirCluster;
                while(clus){
                    clus = fat32_next_cluster(self, context, clus);
                    e++;
                }
                file->size = e*context->sectorsPerCluster*context->bytesPerSector;
            }
            
            file->mode = mode;
            file->flags = flags;
            file->fs = self;
            file->filename = kmalloc(strlen(pathTmp)+1);
            memcpy(file->filename, pathTmp, strlen(pathTmp)+1);
            file->fsData = (void*)fsData;
            fsData->startCluster = currentCluster;

            if(hasEndSlash) path[strlen(path)] = '/';
            return file;
        }

        //If this is a dir, then remove the '/' from the end
        if(path[strlen(path)-1] == '/'){
            path[strlen(path)-1] = 0;
            hasEndSlash = true;
        }

        while(strlen(path)){

            //read next object

            if(path[strptr] != 0 && path[strptr] != '/'){
                strptr++;
                continue;
            }

            for(size_t i = 0; i < strptr; i++){
                dir[i] = path[0];
                path++;
                if(path[0] == 0){
                    isTargetObject = true;
                    break;
                }
                else if(path[0] == '/'){
                    path++;
                    break;
                }
            }
            dir[strptr] = 0;
            
            //search for the current object

            found = false;
            isLFNDirectory = false;
            foundByLFN = false;
            for(;;){
                for(size_t i = 0; i < context->sectorsPerCluster; i++){
                    buf = bcache_read(self->drive, currentSector);
                    for(size_t j = 0; j < context->bytesPerSector; j+=sizeof(fat_directory_t)){
                        currentDir = (fat_directory_t*)((uint64_t)buf->data + j);
                        if(currentDir->name[0] == 0) continue;
                        if(currentDir->attr == FAT_ATTR_LFN) {
                            currentLFN = (fat_lfn_t*)currentDir;
                            isLFNDirectory = true;
                            if(currentLFN->ord == 0x1 || currentLFN->ord == 0x41){
                                if(fat_parse_and_compare_lfn(currentLFN, dir)){
                                    foundByLFN = true;
                                } 
                                continue;
                            }
                            fat_parse_and_compare_lfn(currentLFN, dir);
                        } else if(foundByLFN){
                            found = true;
                            currentCluster = currentDir->clusterLow + (currentDir->clusterHigh << 16);
                            currentSector = partitionOffset + context->firstDataSector + context->sectorsPerCluster * (currentCluster - 2);
                            break;
                        } else if(!isLFNDirectory && fat_parse_and_compare_sfn(currentDir, dir)){
                            found = true;
                            currentCluster = currentDir->clusterLow + (currentDir->clusterHigh << 16);
                            currentSector = partitionOffset + context->firstDataSector + context->sectorsPerCluster * (currentCluster - 2);
                            break;
                        } else {
                            isLFNDirectory = false;
                        }
                    }
                    if(!found) currentSector++;
                    bcache_release(buf);
                }
                if(found) break;
                if(fat32_next_cluster(self, context, currentCluster)){
                    currentCluster = fat32_next_cluster(self, context, currentCluster);
                    currentSector = partitionOffset + context->firstDataSector + context->sectorsPerCluster * (currentCluster - 2);
                } else {
                    //FILE NOT FOUND/BAD PATH
                    kerrno = ENOENT;
                    goto fail;
                }
            }
            if(found && isTargetObject){
                if(((currentDir->attr & FAT_ATTR_DIRECTORY) && !(flags & O_DIRECTORY)) ||
                  ((currentDir->attr & FAT_ATTR_DIRECTORY) && ((flags & O_WRONLY) || (flags & O_RDWR)))){
                    //TARGET IS A DIRECTORY
                    kerrno = EISDIR;
                    goto fail;
                }
                if(!(currentDir->attr & FAT_ATTR_DIRECTORY) && (flags & O_DIRECTORY)){
                    //TARGET IS A FILE
                    kerrno = ENOTDIR;
                    goto fail;
                }
                file_t* file = kmalloc(sizeof(file_t));
                memset(file, 0, sizeof(file_t));
                fat_file_fs_data_t* fsData = kmalloc(sizeof(fat_file_fs_data_t));
                if(flags & O_DIRECTORY){
                    int e = 0;
                    int clus = currentCluster;
                    while(clus){
                        clus = fat32_next_cluster(self, context, clus);
                        e++;
                    }
                    file->size = e*context->sectorsPerCluster*context->bytesPerSector;
                } else {
                    file->size = currentDir->size;
                }
                file->mode = mode;
                file->flags = flags;
                file->fs = self;
                file->filename = kmalloc(strlen(pathTmp)+1);
                memcpy(file->filename, pathTmp, strlen(pathTmp)+1);
                file->fsData = (void*)fsData;

                fsData->startCluster = currentCluster;

                if(hasEndSlash) path[strlen(path)] = '/';
                return file;
            }
            
            strptr = 0;
        }
    }

fail:
    if(hasEndSlash) path[strlen(path)] = '/';
    return NULL;
}