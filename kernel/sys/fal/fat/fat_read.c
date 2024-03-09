/*  
*   File: fat_read.c
*
*   Author: Garnek
*   
*   Description: FAT Driver read routine
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fat.h"
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>
#include <kerrno.h>
#include <kstdio.h>

ssize_t fat_read(filesys_t* self, file_t* file, size_t size, void* buf, size_t offset){
    fat_file_fs_data_t* fsData = (fat_file_fs_data_t*)file->fsData;
    fat_context_t* context = self->context;

    const size_t partitionOffset = self->drive->partitions[self->partition].startLBA; //must be added to read from the correct partition
    size_t currentCluster = fsData->startCluster;
    size_t currentSector = partitionOffset + context->firstDataSector + context->sectorsPerCluster * (currentCluster - 2);

    int j = 0, p = 0;

    //TODO: Use page cache instead of buffer cache

    uint8_t* sectBuf = kmalloc(512);
    if(file->flags & O_DIRECTORY){

        bool isInLFN = false;
        bool LFNParsedFirst = false;
        char* str; //filename
        uint32_t recordLength = 0;
        uint32_t recordType = 0;
        uint64_t recordOffset = 0;
        garn_dirent64_t dirent;

        if(self->type == FILESYS_TYPE_FAT32){
            while(p != size && currentCluster){
                for(int i = 0; i < context->sectorsPerCluster; i++){
                    self->drive->read(self->drive, currentSector+i, 1, sectBuf);
                    for(int k = 0; k < context->bytesPerSector;){
                        fat_directory_t* dir = sectBuf+k;
                        k+=sizeof(fat_directory_t);

                        if(recordOffset < offset){
                            recordOffset += sizeof(fat_directory_t);
                            continue;
                        };

                        if(dir->name[0] == 0) continue;

                        if(dir->attr == FAT_ATTR_LFN){
                            isInLFN == true;
                            str = fat_parse_lfn((fat_lfn_t*)dir);
                            if(str){
                                isInLFN = false;
                                LFNParsedFirst = true;
                            }
                            continue;
                        }

                        if(LFNParsedFirst) LFNParsedFirst = false;
                        else str = fat_parse_sfn(dir);

                        if(dir->attr & FAT_ATTR_DIRECTORY) recordType = DT_DIR;
                        else recordType = DT_REG;

                        recordLength = sizeof(garn_dirent64_t) + strlen(str);

                        dirent.recordLength = recordLength;
                        dirent.recordOffset = recordOffset;
                        dirent.type = recordType;

                        j += sizeof(garn_dirent64_t) + strlen(str);
                        if(j > size){
                            kmfree(str);
                            kmfree(sectBuf);

                            j -= sizeof(garn_dirent64_t) + strlen(str);

                            return j;
                        }

                        memcpy(&buf[p], &dirent, sizeof(garn_dirent64_t)-1);
                        p+=sizeof(garn_dirent64_t)-1;
                        memcpy(&buf[p], str, strlen(str)+1);
                        p+=strlen(str)+1;

                        recordOffset += sizeof(fat_directory_t);

                        recordLength = 0;
                        recordType = 0;
                        kmfree(str);
                    }

                }
                currentCluster = fat32_next_cluster(self, context, currentCluster);
                currentSector = partitionOffset + context->firstDataSector + context->sectorsPerCluster * (currentCluster - 2);
            }
        }
    } else {
        if(self->type == FILESYS_TYPE_FAT32){
            while(p != size && currentCluster){
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
        }
    }

    kmfree(sectBuf);
    
    return p;
}