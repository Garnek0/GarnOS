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

file_t* fat_open(filesys_t* self, char* path, uint8_t access){
    ;
}

uint8_t fat_read(filesys_t* self, file_t* file, size_t size, void* buf){
    ;
}

uint8_t fat_write(filesys_t* self, file_t* file, size_t size, void* buf){
    ;
}

uint8_t fat_close(filesys_t* self, file_t* file){
    ;
}

uint8_t fat_mkdir(filesys_t* self, char* path){
    ;
}

uint8_t fat_rmdir(filesys_t* self, char* path){
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

    //TODO: Actual file operations
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
            klog("FAL: Filesystem on drive \"\" partition %d is corrupt or unsupported!\n", KLOG_WARNING, drive->name, partition);
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
            klog("FAL: Filesystem on drive \"\" partition %d is corrupt or unsupported!\n", KLOG_WARNING, drive->name, partition);
            goto fail;
        }
    } else {
        filesys.type = FILESYS_TYPE_FAT32;
        context->ebpb = (void*)fat32ebpb;
        filesys.context = (void*)context;
        if(fat32ebpb->fatVer != 0){
            klog("FAL: Filesystem on drive \"\" partition %d is corrupt or unsupported!\n", KLOG_WARNING, drive->name, partition);
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
            klog("FAL: Filesystem on drive \"\" partition %d is corrupt or unsupported!\n", KLOG_WARNING, drive->name, partition);
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