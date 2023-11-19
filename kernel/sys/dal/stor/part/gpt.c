/*  
*   File: gpt.c
*
*   Author: Garnek
*   
*   Description: GUID Partition Table Parsing
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "gpt.h"
#include <mem/mm/kheap.h>
#include <mem/memutil/memutil.h>
#include <cpu/smp/spinlock.h>

spinlock_t gptLock;

bool gpt_validate_drive(drive_t* drive){

    if(drive->type == DRIVE_TYPE_OPTICAL) return false;

    gpt_header_t* partHeader = (gpt_header_t*)kmalloc(512);

    lock(gptLock, {
        drive->read(drive, 1, 512, (void*)partHeader);

        if(!strcmp(partHeader->magic, "EFI PART") || !partHeader->entrySize > 512 || (partHeader->entrySize)%128 != 0 || partHeader->partCount > MAX_PARTITIONS){
            kmfree(partHeader);
            releaseLock(&gptLock);
            return true;
        }
        kmfree(partHeader);
        releaseLock(&gptLock);
        return false;
    });
}

bool gpt_initialise_drive(drive_t* drive){
    gpt_header_t* partHeader = (gpt_header_t*)kmalloc(512);
    void* partEntryChunk = (gpt_header_t*)kmalloc(512);
    gpt_entry_t* currentEntry;

    lock(gptLock, {
        drive->read(drive, 1, 512, (void*)partHeader);
        if(!partHeader->gptArrayStartLBA) partHeader->gptArrayStartLBA = 2;
        
        for(int i = 0; i < (partHeader->partCount/(512/(partHeader->entrySize))); i++){
            drive->read(drive, partHeader->gptArrayStartLBA + i, 512, (void*)partEntryChunk);
            for(int j = 0; j < 512/(partHeader->entrySize); j++){
                currentEntry = (gpt_entry_t*)((uint64_t)partEntryChunk + (j*partHeader->entrySize));
                if(!currentEntry->typeGUID[0] == 0 && !currentEntry->typeGUID[1] == 0){
                    drive->partitionCount++;
                    for(size_t i = 0; i < MAX_PARTITIONS; i++){
                        if(!drive->partitions[i]._valid){
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
        }

        kmfree(partHeader);
        kmfree(partEntryChunk);
        releaseLock(&gptLock);
        return true;
    });
}