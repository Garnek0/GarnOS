/*  
*   File: gpt.c
*
*   Author: Garnek
*   
*   Description: GUID Partition Table Parsing
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "gpt.h"
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>
#include <cpu/multiproc/spinlock.h>
#include <sys/bootloader.h>
#include <sys/fal/fal.h>

spinlock_t gptLock;

bool gpt_validate_drive(drive_t* drive){
    if(drive->type == DRIVE_TYPE_OPTICAL) return false;

    bcache_buf_t* buf;

    gpt_header_t* partHeader;

    lock(gptLock, {
        buf = bcache_read(drive, 1);
        partHeader = (gpt_header_t*)buf->data;

        uint64_t* bootDiskGUID = bl_get_gpt_system_disk_uuid();
        if(!strcmp(partHeader->magic, "EFI PART") || !partHeader->entrySize > 512 || (partHeader->entrySize)%128 != 0 || partHeader->partCount > MAX_PARTITIONS){
            if(bootDiskGUID[0] == partHeader->GUID[0] && bootDiskGUID[1] == partHeader->GUID[1]) drive->isSystemDrive = true;
            bcache_release(buf);
            releaseLock(&gptLock);
            return true;
        }
        bcache_release(buf);
        releaseLock(&gptLock);
        return false;
    });
}

bool gpt_initialise_drive(drive_t* drive){
    bcache_buf_t* buf;
    gpt_header_t* partHeader = kmalloc(sizeof(gpt_header_t));
    void* partEntryChunk;
    gpt_entry_t* currentEntry;

    lock(gptLock, {
        buf = bcache_read(drive, 1);
        memcpy(partHeader, buf->data, sizeof(gpt_header_t));
        bcache_release(buf);    

        if(!partHeader->gptArrayStartLBA) partHeader->gptArrayStartLBA = 2;
        
        uint64_t* bootPartGUID = bl_get_gpt_system_partition_uuid();
        for(int i = 0; i < (partHeader->partCount/(512/(partHeader->entrySize))); i++){
            buf = bcache_read(drive, partHeader->gptArrayStartLBA + i);
            partEntryChunk = buf->data;
            for(int j = 0; j < 512/(partHeader->entrySize); j++){
                currentEntry = (gpt_entry_t*)((uint64_t)partEntryChunk + (j*partHeader->entrySize));
                if(!currentEntry->typeGUID[0] == 0 && !currentEntry->typeGUID[1] == 0){
                    drive->partitionCount++;
                    for(size_t i = 0; i < MAX_PARTITIONS; i++){
                        if(!drive->partitions[i]._valid){
                            if(currentEntry->partGUID[0] == bootPartGUID[0] && currentEntry->partGUID[0] == bootPartGUID[0] && drive->isSystemDrive) drive->partitions[i].isSystemPartition = true;
                            drive->partitions[i].startLBA = currentEntry->startLBA;
                            drive->partitions[i].endLBA = currentEntry->endLBA;
                            drive->partitions[i].attribs = currentEntry->attribs;
                            drive->partitions[i].size = (currentEntry->endLBA - currentEntry->startLBA)*512;
                            drive->partitions[i]._valid = true;
                            break;
                        }
                    }
                    klog("DAL: Found Partition of Drive \"%s\". (startLBA: 0x%x, endLBA: 0x%x)\n", KLOG_OK, drive->name, currentEntry->startLBA, currentEntry->endLBA);
                }
            }
            bcache_release(buf);
        }

        kmfree(partHeader);
        releaseLock(&gptLock);
        return true;
    });
}