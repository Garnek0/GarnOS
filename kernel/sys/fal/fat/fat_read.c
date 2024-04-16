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

    if(offset >= file->size) return 0;

    uint8_t* sectBuf = kmalloc(512);
    if(file->flags & O_DIRECTORY){

        int bufind = 0;

        bool isInLFN = false;
        bool LFNParsedFirst = false;
        char* str; //filename
        uint32_t recordLength = 0;
        uint32_t recordType = 0;
        uint64_t recordOffset = 0;
        dirent_t dirent;

        fat_file_fs_data_t* fsData = (fat_file_fs_data_t*)file->fsData;
        dirent.inode = fsData->startCluster;

        size_t trueOffset = 0;

        if(!strcmp(self->type, FILESYS_TYPE_FAT32)){
            while(currentCluster){
                for(int i = 0; i < context->sectorsPerCluster; i++){
                    self->drive->read(self->drive, currentSector+i, 1, sectBuf);
                    for(int k = 0; k < context->bytesPerSector;){
                        if(trueOffset >= file->size) goto done;
                        fat_directory_t* dir = sectBuf+k;
                        k+=sizeof(fat_directory_t);

                        if(recordOffset < trueOffset){
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
                            trueOffset += sizeof(fat_directory_t);
                            recordOffset += sizeof(fat_directory_t);
                            continue;
                        }

                        if(LFNParsedFirst) LFNParsedFirst = false;
                        else str = fat_parse_sfn(dir);

                        if(dir->attr & FAT_ATTR_DIRECTORY) recordType = DT_DIR;
                        else recordType = DT_REG;

                        recordLength = sizeof(dirent_t) + strlen(str);
                        p += recordLength;

                        if(p == offset){
                            j = 0;
                            for(size_t g = 0; g < size; g++){
                                ((uint8_t*)buf)[g] = 0;
                            }

                            bufind = 0;

                            kmfree(str);
                            continue;
                        }

                        j += sizeof(dirent_t) + strlen(str);
                        if(j > size){
                            if(offset > p){
                                j = recordLength;
                                for(size_t g = 0; g < size; g++){
                                    ((uint8_t*)buf)[g] = 0;
                                }

                                bufind = 0;
                            } else {
                                kmfree(str);
                                kmfree(sectBuf);

                                j -= sizeof(dirent_t) + strlen(str);

                                return j;
                            }
                            
                        }

                        dirent.reclen = recordLength;
                        dirent.offset = recordOffset;
                        dirent.type = recordType;

                        memcpy(&buf[bufind], &dirent, sizeof(dirent_t)-1);
                        bufind+=sizeof(dirent_t)-1;
                        memcpy(&buf[bufind], str, strlen(str)+1);
                        bufind+=strlen(str)+1;

                        recordOffset += sizeof(fat_directory_t);
                        trueOffset += sizeof(fat_directory_t);

                        recordLength = 0;
                        recordType = 0;
                        kmfree(str);
                    }

                }
                currentCluster = fat32_next_cluster(self, context, currentCluster);
                currentSector = partitionOffset + context->firstDataSector + context->sectorsPerCluster * (currentCluster - 2);
            }
            p = j;
        }
    } else {
        if(!strcmp(self->type, FILESYS_TYPE_FAT32)){
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

done:
    kmfree(sectBuf);
    return p;
}