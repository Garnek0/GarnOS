/*  
*   File: fat_helper.c
*
*   Author: Garnek
*   
*   Description: FAT Driver helper functions
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fat.h"
#include <garn/mm.h>
#include <garn/kerrno.h>
#include <garn/kstdio.h>

//TODO: Support sector sizes != 512 Bytes ???????

size_t fat12_next_cluster(vfs_t* ilesys, fat_context_t* context, size_t cluster){
    //TODO: Do this
    return 0;
}

size_t fat16_next_cluster(vfs_t* vfs, fat_context_t* context, size_t cluster){
    size_t sectorOffset = cluster/256;

    bcache_buf_t* buf = bcache_get(vfs->drive, vfs->drive->partitions[vfs->partition].startLBA + context->firstFATSector + sectorOffset);
	if(!buf){
		klog("Could not get next cluster!\n", KLOG_FAILED, "FAT");
		return 0;
	}

    uint16_t* fatBuf = (uint16_t*)buf->data;

    if(fatBuf[cluster%256] >= 0xFFF6){
        bcache_release(buf);
        return 0;
    }
    bcache_release(buf);
    return fatBuf[cluster%256];
}

size_t fat32_next_cluster(vfs_t* vfs, fat_context_t* context, size_t cluster){
    size_t sectorOffset = cluster/128;

    bcache_buf_t* buf = bcache_get(vfs->drive, vfs->drive->partitions[vfs->partition].startLBA + context->firstFATSector + sectorOffset);
	if(!buf){
		klog("Could not get next cluster!\n", KLOG_FAILED, "FAT");
		return 0;
	}

    uint32_t* fatBuf = (uint32_t*)buf->data;

    if(fatBuf[cluster%128] >= 0x0FFFFF6){
        bcache_release(buf);
        return 0;
    }
    bcache_release(buf);
    return fatBuf[cluster%128];
}

char* fat_parse_sfn(fat_directory_t* sd1){
    char* buf = kmalloc(13);

    uint8_t strptr = 0;
    for(uint8_t i = 0; i < 8; i++){
        if(sd1->name[i] == ' ') break;
        buf[strptr] = sd1->name[i];
        strptr++;
    }
    if(buf[strptr-1]!='.' && sd1->name[8]!=' ') buf[strptr] = '.';
    if(sd1->attr != FAT_ATTR_DIRECTORY) strptr++;
    for(uint8_t i = 8; i < 11; i++){
        if(sd1->name[i] == ' ') break;
        buf[strptr] = sd1->name[i];
        strptr++;
    }
    buf[strptr] = 0;
    
    return buf;
}

bool fat_parse_and_compare_sfn(fat_directory_t* sd1, char* s2){
    bool comp;
    char* s1 = fat_parse_sfn(sd1);

    if(!strcmp(s1, s2)) comp = true;
    else comp = false;

    kmfree(s1);

    return comp;
}

uint8_t LFNCharIndex;
bool LFNNewEntry = true;
char LFNWorkBuffer[256];
char* fat_parse_lfn(fat_lfn_t* lfn){
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
            klog("corrupt LFN\n", KLOG_FAILED, "FAT");
            return false;
        }
        char* buf = kmalloc(256);
        memcpy(buf, &LFNWorkBuffer[LFNCharIndex], strlen(&LFNWorkBuffer[LFNCharIndex])+1);
        return buf;
    }
    return NULL;

}

bool fat_parse_and_compare_lfn(fat_lfn_t* lfn, char* s2){
    char* s1 = fat_parse_lfn(lfn);
    if(!s1) return false;

    bool comp;

    if(!strcmp(s1, s2)) comp = true;
    else comp = false;

    kmfree(s1);

    return comp;
}
