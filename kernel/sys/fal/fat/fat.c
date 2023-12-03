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
#include <mem/mm/kheap.h>
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

uint8_t* clusterCalcBuf[512];

static size_t fat12_next_cluster(filesys_t* filesys, fat_context_t* context, size_t cluster){
    //TODO: Do this
}

static size_t fat16_next_cluster(filesys_t* filesys, fat_context_t* context, size_t cluster){
    uint16_t* buf = (uint16_t*)clusterCalcBuf;
    size_t sectorOffset = cluster/256;

    filesys->drive->read(filesys->drive, filesys->drive->partitions[filesys->partition].startLBA + context->firstFATSector + sectorOffset, 1, buf);
    if(buf[cluster%256] >= 0xFFF6) return 0;
    return buf[cluster%256];
}

static size_t fat32_next_cluster(filesys_t* filesys, fat_context_t* context, size_t cluster){
    uint32_t* buf = (uint32_t*)clusterCalcBuf;
    size_t sectorOffset = cluster/128;

    filesys->drive->read(filesys->drive, filesys->drive->partitions[filesys->partition].startLBA + context->firstFATSector + sectorOffset, 1, buf);
    if(buf[cluster%128] >= 0x0FFFFF6) return 0;
    return buf[cluster%128];
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

file_t* fat_open(filesys_t* self, char* path, uint8_t access){
    fat_context_t* context = (fat_context_t*)self->context;
    fat_directory_t* currentDir;
    fat_lfn_t* currentLFN;
    void* buf = kmalloc(512);
    char* pathTmp = path;

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
                for(int i = 0; i < fat32ebpb->bpb.sectsPerCluster; i++){
                    self->drive->read(self->drive, currentSector, 1, buf);
                    for(int j = 0; j < fat32ebpb->bpb.bytesPerSector; j+=sizeof(fat_directory_t)){
                        currentDir = (fat_directory_t*)((uint64_t)buf + j);
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
                            currentSector = partitionOffset + context->firstDataSector + fat32ebpb->bpb.sectsPerCluster * (currentCluster - 2);
                            break;
                        } else if(!isLFNDirectory && fat_parse_and_compare_sfn(currentDir, dir)){
                            found = true;
                            currentCluster = currentDir->clusterLow + (currentDir->clusterHigh << 16);
                            currentSector = partitionOffset + context->firstDataSector + fat32ebpb->bpb.sectsPerCluster * (currentCluster - 2);
                            break;
                        }
                    }
                    if(!found) currentSector++;
                }
                if(found) break;
                if(fat32_next_cluster(self, context, currentCluster)){
                    currentCluster = fat32_next_cluster(self, context, currentCluster);
                    currentSector = partitionOffset + context->firstDataSector + fat32ebpb->bpb.sectsPerCluster * (currentCluster - 2);
                } else {
                    //FILE NOT FOUND/BAD PATH
                    klog("%s: File Not Found.\n", KLOG_FAILED, pathTmp);
                    kerrno = ENOENT;
                    return NULL;
                }
            }
            if(found && isTargetObject){
                if(currentDir->attr & FAT_ATTR_DIRECTORY){
                    //TARGET IS A DIRECTORY
                    klog("%s: Is a directory.\n", KLOG_FAILED, pathTmp);
                    kerrno = EISDIR;
                }
                file_t* file = kmalloc(sizeof(file_t));
                file->size = currentDir->size;
                file->access = access;
                file->fs = self;
                file->filename = kmalloc(strlen(pathTmp));
                memcpy(file->filename, pathTmp, strlen(pathTmp));
                file->seek = 0;
                file->address = kmalloc(ALIGN_UP(file->size, (fat32ebpb->bpb.bytesPerSector*fat32ebpb->bpb.sectsPerCluster)));

                int j = 0;

                do {
                    for(int i = 0; i < fat32ebpb->bpb.sectsPerCluster; i++){
                        self->drive->read(self->drive, currentSector+i, 1, buf);
                        memcpy(file->address+j, buf, fat32ebpb->bpb.bytesPerSector);
                        j += fat32ebpb->bpb.bytesPerSector;
                    }
                    currentCluster = fat32_next_cluster(self, context, currentCluster);
                    currentSector = partitionOffset + context->firstDataSector + fat32ebpb->bpb.sectsPerCluster * (currentCluster - 2);
                } while(currentCluster);

                return file;
            }
            
            strptr = 0;
        }
    }

    //while(1);
    return NULL;
}

int fat_read(filesys_t* self, file_t* file, size_t size, void* buf){
    for(size_t i = 0; i < size; i++){
        if(file->seek >= file->size) {file->seek--; return i;}
        ((uint8_t*)buf)[i] = ((uint8_t*)file->address)[file->seek];
        file->seek++;
    }
    return size;
}

int fat_write(filesys_t* self, file_t* file, size_t size, void* buf){
    ;
}

int fat_close(filesys_t* self, file_t* file){
    kmfree(file->address);
    kmfree(file);
}

int fat_mkdir(filesys_t* self, char* path){
    ;
}

int fat_rmdir(filesys_t* self, char* path){
    ;
}


bool fat_probe(drive_t* drive, size_t partition){
    void* buf = kmalloc(512);
    fat_bpb_t* probeBPB;

    drive->read(drive, drive->partitions[partition].startLBA, 1, buf);
    probeBPB = (fat_bpb_t*)buf;

    if(probeBPB->jump[0] != 0xEB && probeBPB->jump[2] != 0x90)  goto probefail;

    for(int i = 0; i < PROBE_OEMID_COUNT; i++){
        if(!strncmp((const char*)probeBPB->OEMID, probeOEMIDs[i], 8)){
            //This is surely a FAT filesystem
            goto probesuccess;
        }
    }

    goto probesuccess;

probefail:
    kmfree(buf);
    return false;

probesuccess:
    kmfree(buf);
    return true;
}

bool fat_attach(drive_t* drive, size_t partition){
    if(!fat_probe(drive, partition)) return false;\

    //TODO: Move to module

    void* buf = kmalloc(512);
    fat_context_t* context = kmalloc(sizeof(fat_context_t));
    fat_bpb_t* bpb;
    fat12_16_ebpb_t* fat12_16ebpb;
    fat32_ebpb_t* fat32ebpb;

    drive->read(drive, drive->partitions[partition].startLBA, 1, buf);
    bpb = (fat_bpb_t*)buf;
    fat12_16ebpb = (fat12_16_ebpb_t*)buf;
    fat32ebpb = (fat32_ebpb_t*)buf;

    filesys_t filesys;
    filesys.drive = drive;
    filesys.partition = partition;
    filesys.open = fat_open;
    filesys.close = fat_close;
    filesys.write = fat_write;
    filesys.read = fat_read;
    filesys.mkdir = fat_mkdir;
    filesys.rmdir = fat_rmdir;

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

    //correct size
    filesys.size = context->dataSectors*512;

    if(context->clusterCount < 4085){
        filesys.type = FILESYS_TYPE_FAT12;
        context->ebpb = (void*)fat12_16ebpb;
        filesys.context = (void*)context;
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
    kmfree(buf);
    return false;

success:

    filesys_mount(filesys);

    return true;
}
