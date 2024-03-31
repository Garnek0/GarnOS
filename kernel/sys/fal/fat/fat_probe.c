/*  
*   File: fat_probe.c
*
*   Author: Garnek
*   
*   Description: FAT Driver probe and attach routines
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fat.h"
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>
#include <kerrno.h>
#include <kstdio.h>

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
    context->rootDirSectors = ((bpb->rootDirEntryCount * 32) + (bpb->bytesPerSector - 1)) / bpb->bytesPerSector; //FAT12 and FAT16 only
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