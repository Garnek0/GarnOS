/*  
*   File: fat.c
*
*   Author: Garnek
*   
*   Description: FAT-12/16/32 File System Implementations. This
*                FS driver should probably be moved to a separate
*                module, but for now, to keep things simple it is 
*                part of the FAL
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fat.h"
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>
#include <kerrno.h>

#define PROBE_OEMID_COUNT 15

const char* probeOEMIDs[PROBE_OEMID_COUNT] = {
    "GARNFAT ", //Garn Kernel custom OEMID
    "MSDOS2.0",
    "MSDOS2.1",
    "MSDOS3.0",
    "MSDOS3.1",
    "MSDOS3.2",
    "MSDOS3.3",
    "MSDOS4.0",
    "MSDOS5.0",
    "MSDOS5.1",
    "MSWIN4.0",
    "MSWIN4.1",
    "FRDOS5.1",
    "mkdosfs",
    "mkfs.fat"
};


//TODO: Support sector sizes != 512 Bytes ???????

static size_t fat12_next_cluster(filesys_t* filesys, fat_context_t* context, size_t cluster){
    //TODO: Do this
}

static size_t fat16_next_cluster(filesys_t* filesys, fat_context_t* context, size_t cluster){
    size_t sectorOffset = cluster/256;

    bcache_buf_t* buf = bcache_read(filesys->drive, filesys->drive->partitions[filesys->partition].startLBA + context->firstFATSector + sectorOffset);

    uint16_t* fatBuf = (uint16_t*)buf->data;

    if(fatBuf[cluster%256] >= 0xFFF6){
        bcache_release(buf);
        return 0;
    }
    bcache_release(buf);
    return fatBuf[cluster%256];
}

static size_t fat32_next_cluster(filesys_t* filesys, fat_context_t* context, size_t cluster){
    size_t sectorOffset = cluster/128;

    bcache_buf_t* buf = bcache_read(filesys->drive, filesys->drive->partitions[filesys->partition].startLBA + context->firstFATSector + sectorOffset);

    uint32_t* fatBuf = (uint32_t*)buf->data;

    if(fatBuf[cluster%128] >= 0x0FFFFF6){
        bcache_release(buf);
        return 0;
    }
    bcache_release(buf);
    return fatBuf[cluster%128];
}

static bool fat_parse_and_compare_sfn(fat_directory_t* sd1, char* s2){
    char tmp[13];
    uint8_t strptr = 0;
    for(uint8_t i = 0; i < 8; i++){
        if(sd1->name[i] == ' ') break;
        tmp[strptr] = sd1->name[i];
        strptr++;
    }
    if(tmp[strptr-1]!='.' && sd1->name[8]!=' ') tmp[strptr] = '.';
    if(sd1->attr != FAT_ATTR_DIRECTORY) strptr++;
    for(uint8_t i = 8; i < 11; i++){
        if(sd1->name[i] == ' ') break;
        tmp[strptr] = sd1->name[i];
        strptr++;
    }
    tmp[strptr] = 0;
    if(!strcmp(tmp, s2)) return true;
    return false;
}

static char LFNWorkBuffer[256];
uint8_t LFNCharIndex;
bool LFNNewEntry = true;
static bool fat_parse_and_compare_lfn(fat_lfn_t* lfn, char* s2){
    if(LFNNewEntry){
        LFNNewEntry = false;
        LFNCharIndex = 255;
    }

    for(uint8_t i = 0; i < 2; i++){
        if(lfn->name3[1-i] == 0xFFFF) continue;

        LFNWorkBuffer[LFNCharIndex] = (char)(lfn->name3[1-i] & 0xFF);
        LFNCharIndex--;
    }
    for(uint8_t i = 0; i < 6; i++){
        if(lfn->name2[5-i] == 0xFFFF) continue;

        LFNWorkBuffer[LFNCharIndex] = (char)(lfn->name2[5-i] & 0xFF);
        LFNCharIndex--;
    }
    for(uint8_t i = 0; i < 5; i++){
        if(lfn->name1[4-i] == 0xFFFF) continue;

        LFNWorkBuffer[LFNCharIndex] = (char)(lfn->name1[4-i] & 0xFF);
        LFNCharIndex--;
    }

    if(lfn->ord == 0x1 || lfn->ord == 0x41){
        LFNCharIndex++;
        LFNNewEntry = true;
        if(LFNWorkBuffer[255] != 0){
            //assume the name is corrupt
            klog("FAT: corrupt LFN\n", KLOG_FAILED);
            return false;
        }

        if(!strcmp(&LFNWorkBuffer[LFNCharIndex], s2)) return true;
        return false;
    }
}

file_t* fat_open(filesys_t* self, char* path, int flags, int mode){
    fat_context_t* context = (fat_context_t*)self->context;
    fat_directory_t* currentDir;
    fat_lfn_t* currentLFN;
    bcache_buf_t* buf;
    bool hasEndSlash = false;
    char* pathTmp = kmalloc(strlen(path)+1);
    memcpy(pathTmp, path, strlen(path)+1);

    if(self->type == FILESYS_TYPE_FAT12){
        //TODO: This      
    } else if(self->type == FILESYS_TYPE_FAT16){
        //TODO: This
    } else {
        fat32_ebpb_t* fat32ebpb = (fat32_ebpb_t*)context->ebpb;
        
        char name[256]; //holds the filename of the opened file
        char dir[256]; //holds the filename of the current object
        bool isTargetObject = false;
        size_t strptr = 0; //used for getting filenames from the path string

        const size_t partitionOffset = self->drive->partitions[self->partition].startLBA; //must be added to read from the correct partition
        size_t currentSector = partitionOffset + context->firstRootDirSector;
        size_t currentSectorIndex = 0; //needed to be able to tell when to read a new sector;
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
            file->size = context->rootDirSectors * context->bytesPerSector;
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

            if(path[strptr] != NULL && path[strptr] != '/'){
                strptr++;
                continue;
            }

            for(int i = 0; i < strptr; i++){
                dir[i] = path[0];
                path++;
                if(path[0] == NULL){
                    isTargetObject = true;
                    break;
                }
                else if(path[0] == '/'){
                    path++;
                    break;
                }
            }
            dir[strptr] = NULL;
            
            //search for the current object

            found = false;
            isLFNDirectory = false;
            foundByLFN = false;
            for(;;){
                for(int i = 0; i < context->sectorsPerCluster; i++){
                    buf = bcache_read(self->drive, currentSector);
                    for(int j = 0; j <context->bytesPerSector; j+=sizeof(fat_directory_t)){
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
                file->size = currentDir->size;
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

ssize_t fat_read(filesys_t* self, file_t* file, size_t size, void* buf, size_t offset){
    fat_file_fs_data_t* fsData = (fat_file_fs_data_t*)file->fsData;
    fat_context_t* context = self->context;

    const size_t partitionOffset = self->drive->partitions[self->partition].startLBA; //must be added to read from the correct partition
    size_t currentCluster = fsData->startCluster;
    size_t currentSector = partitionOffset + context->firstDataSector + context->sectorsPerCluster * (currentCluster - 2);

    int j = 0, p = 0;

    //TODO: Use page cache instead of buffer cache

    uint8_t* sectBuf = kmalloc(512);

    while(p != size && currentSector){
        for(int i = 0; i < context->sectorsPerCluster; i++){
            self->drive->read(self->drive, currentSector+i, 1, sectBuf);
            for(int k = 0; k < context->bytesPerSector; k++){
                if(j < offset){
                    j++;
                    continue;
                }
                ((uint8_t*)buf)[p] = sectBuf[k];
                offset++;
                p++; j++;
                if(p == size) break;
            }
        }
        currentCluster = fat32_next_cluster(self, context, currentCluster);
        currentSector = partitionOffset + context->firstDataSector + context->sectorsPerCluster * (currentCluster - 2);
    }

    kmfree(sectBuf);
    
    return j;
}

ssize_t fat_write(filesys_t* self, file_t* file, size_t size, void* buf, size_t offset){
    ;
}

int fat_close(filesys_t* self, file_t* file){
    if(file->fsData) kmfree(file->fsData);
    kmfree(file);
}

int fat_mkdir(filesys_t* self, char* path){
    ;
}

int fat_rmdir(filesys_t* self, char* path){
    ;
}


bool fat_probe(drive_t* drive, size_t partition){
    fat_bpb_t* probeBPB;

    bcache_buf_t* buf = bcache_read(drive, drive->partitions[partition].startLBA);

    probeBPB = (fat_bpb_t*)buf->data;

    if(probeBPB->jump[0] != 0xEB && probeBPB->jump[2] != 0x90) {
        bcache_release(buf);
        return false;
    }

    for(int i = 0; i < PROBE_OEMID_COUNT; i++){
        if(!strncmp((const char*)probeBPB->OEMID, probeOEMIDs[i], 8)){
            //This is surely a FAT filesystem
            bcache_release(buf);
            return true;
        }
    }

    bcache_release(buf);
    return false;
}

bool fat_attach(drive_t* drive, size_t partition){
    if(!fat_probe(drive, partition)) return false;\

    //TODO: Move to module

    bcache_buf_t* buf;
    fat_context_t* context = kmalloc(sizeof(fat_context_t));
    fat_bpb_t* bpb;
    fat12_16_ebpb_t* fat12_16ebpb;
    fat32_ebpb_t* fat32ebpb;

    buf = bcache_read(drive, drive->partitions[partition].startLBA);
    bpb = kmalloc(sizeof(fat_bpb_t));
    fat12_16ebpb = kmalloc(sizeof(fat12_16_ebpb_t));
    fat32ebpb = kmalloc(sizeof(fat32_ebpb_t));
    memcpy(bpb, (void*)buf->data, sizeof(fat_bpb_t));
    memcpy(fat12_16ebpb, (void*)buf->data, sizeof(fat12_16_ebpb_t));
    memcpy(fat32ebpb, (void*)buf->data, sizeof(fat32_ebpb_t));

    filesys_t filesys;
    filesys.drive = drive;
    filesys.partition = partition;
    filesys.fsOperations.open = fat_open;
    filesys.fsOperations.close = fat_close;
    filesys.fsOperations.write = fat_write;
    filesys.fsOperations.read = fat_read;
    filesys.fsOperations.mkdir = fat_mkdir;
    filesys.fsOperations.rmdir = fat_rmdir;

    if(bpb->bytesPerSector != 512){
        klog("FAT: Filesystem on drive \"%s\" partition %d has an unsupported sector size of %d Bytes per secotr!\n", KLOG_WARNING, drive->name, partition, bpb->bytesPerSector);
        goto fail;
    }

    if(bpb->totalSects16) filesys.size = bpb->totalSects16*bpb->bytesPerSector;
    else filesys.size = bpb->totalSects32*bpb->bytesPerSector;

    context->totalSectors = filesys.size/bpb->bytesPerSector;
    context->FATSize = (bpb->sectsPerFAT == 0) ? fat32ebpb->sectsPerFAT : bpb->sectsPerFAT;
    context->rootDirSectors = ((bpb->rootDirEntryCount * 32) + (bpb->bytesPerSector - 1)) / bpb->bytesPerSector;
    context->firstDataSector = bpb->reservedSectors + (context->FATSize * bpb->FATCount) + context->rootDirSectors;
    context->firstRootDirSector = bpb->reservedSectors + (context->FATSize * bpb->FATCount);
    context->firstFATSector = bpb->reservedSectors;
    context->dataSectors = context->totalSectors - (bpb->reservedSectors + (bpb->FATCount * context->FATSize) + context->rootDirSectors);
    context->clusterCount = (filesys.size - (bpb->reservedSectors + context->FATSize + context->rootDirSectors))/bpb->sectsPerCluster;
    context->clusterSize = bpb->sectsPerCluster * bpb->bytesPerSector;
    context->sectorsPerCluster = bpb->sectsPerCluster;
    context->bytesPerSector = bpb->bytesPerSector;

    //correct size
    filesys.size = context->dataSectors*512;

    if(context->clusterCount < 4085){
        filesys.type = FILESYS_TYPE_FAT12;
        context->ebpb = (void*)fat12_16ebpb;
        filesys.context = (void*)context;
        kmfree(fat32ebpb);
        if(fat12_16ebpb->signature == 0x28){
            filesys_set_name(&filesys, "No Label");

            goto success;
        } else if(fat12_16ebpb->signature == 0x29){
            //volLabel is not null terminated so it needs to be set manually
            char tempLabel[11];
            memcpy(tempLabel, fat12_16ebpb->volLabel, 11);

            for(uint8_t i = 10; i > 0; i--){
                if(tempLabel[i] == 0x20) tempLabel[i] = 0;
                else break;
            }

            memcpy(filesys.name, tempLabel, 11);

            goto success;
        } else {
            klog("FAT: Filesystem on drive \"\" partition %d is corrupt or unsupported!\n", KLOG_WARNING, drive->name, partition);
            goto fail;
        }
    } else if(context->clusterCount < 65525){
        filesys.type = FILESYS_TYPE_FAT16;
        context->ebpb = (void*)fat12_16ebpb;
        filesys.context = (void*)context;
        kmfree(fat32ebpb);
        if(fat12_16ebpb->signature == 0x28){
            filesys_set_name(&filesys, "No Label");

            goto success;
        } else if(fat12_16ebpb->signature == 0x29){
            //volLabel is not null terminated so it needs to be set manually
            char tempLabel[11];
            memcpy(tempLabel, fat12_16ebpb->volLabel, 11);

            for(uint8_t i = 10; i > 0; i--){
                if(tempLabel[i] == 0x20) tempLabel[i] = 0;
                else break;
            }

            memcpy(filesys.name, tempLabel, 11);

            goto success;
        } else {
            klog("FAT: Filesystem on drive \"%s\" partition %d is corrupt or unsupported!\n", KLOG_WARNING, drive->name, partition);
            goto fail;
        }
    } else {
        filesys.type = FILESYS_TYPE_FAT32;
        context->ebpb = (void*)fat32ebpb;
        filesys.context = (void*)context;
        kmfree(fat12_16ebpb);
        if(fat32ebpb->fatVer != 0){
            klog("FAT: Filesystem on drive \"%s\" partition %d is corrupt or unsupported!\n", KLOG_WARNING, drive->name, partition);
            goto fail;
        }
        if(fat32ebpb->signature == 0x28){
            filesys_set_name(&filesys, "No Label");

            goto success;
        } else if(fat32ebpb->signature == 0x29){
            //volLabel is not null terminated so it needs to be set manually
            char tempLabel[11];
            memcpy(tempLabel, fat32ebpb->volLabel, 11);

            for(uint8_t i = 10; i > 0; i--){
                if(tempLabel[i] == 0x20) tempLabel[i] = 0;
                else break;
            }

            memcpy(filesys.name, tempLabel, 11);

            goto success;
        } else {
            klog("FAT: Filesystem on drive \"\" partition %d is corrupt or unsupported!\n", KLOG_WARNING, drive->name, partition);
            goto fail;
        }
    }

fail:
    bcache_release(buf);
    return false;

success:
    bcache_release(buf);
    filesys_mount(filesys);
    return true;
}
