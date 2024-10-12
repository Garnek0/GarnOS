/*
*   Module: fat.sys
*
*   File: file.c
*
*   Module Author: Garnek
*
*   Module Description: FAT12/16/32 Driver
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fat.h"
#include <garn/kerrno.h>
#include <garn/kstdio.h>
#include <garn/dal/bcache.h>
#include <garn/mm.h>

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

char* fat_parse_sfn(fat_directory_t* sd1){
    char* buf = kmalloc(13);

    uint8_t strptr = 0;
    for(uint8_t i = 0; i < 8; i++){
        if(sd1->name[i] == ' ') break;
        buf[strptr] = sd1->name[i];
        strptr++;
    }
    if(buf[strptr-1]!='.' && sd1->name[8]!=' ') buf[strptr] = '.';
    if(!(sd1->attrib & FAT_ATTR_DIRECTORY)) strptr++;
    for(uint8_t i = 8; i < 11; i++){
        if(sd1->name[i] == ' ') break;
        buf[strptr] = sd1->name[i];
        strptr++;
    }
	buf[strptr] = 0;
    
    return buf;
}

size_t fat16_next_cluster(vfs_t* vfs, fat_metadata_t* metadata, size_t cluster){
    size_t sectorOffset = cluster/256;

    bcache_buf_t* buf = bcache_get(vfs->drive, vfs->drive->partitions[vfs->partition].startLBA + metadata->firstFATSector + sectorOffset);
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

size_t fat32_next_cluster(vfs_t* vfs, fat_metadata_t* metadata, size_t cluster){
    size_t sectorOffset = cluster/128;

	bcache_buf_t* buf = bcache_get(vfs->drive, vfs->drive->partitions[vfs->partition].startLBA + metadata->firstFATSector + sectorOffset);
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

vnode_t* fat_lookup(vnode_t* self, const char* name){
	if(!self || self->type != V_DIR){
		kerrno = ENOTDIR;
		return NULL;
	}

	char fullname[256];

	memcpy(fullname, self->filename, strlen(self->filename)+1);
	memcpy(&fullname[strlen(self->filename)], name, strlen(name)+1);	

	vnode_t* listEntry = vnode_list_search_by_filename(self->vfs, fullname);
	if(listEntry){
		return listEntry;
	}

	size_t partitionOffset = self->vfs->drive->partitions[self->vfs->partition].startLBA;
	fat_metadata_t* metadata = (fat_metadata_t*)self->vfs->context;

	bcache_buf_t* buf;
	fat_directory_t* directories;
	fat_lfn_t* lfn;

	size_t currentCluster = ((fat_vnode_data_t*)self->fsData)->firstCluster;
	size_t currentSector;

	bool foundByLFN = false;
	bool isLFN = false;

	char* retrievedName;

	char lookFor[256];
	memcpy(lookFor, name, strlen(name)+1);

	if(lookFor[strlen(lookFor)-1] == '/') lookFor[strlen(lookFor)-1] = 0;

	if(!strcmp(self->vfs->type, FILESYS_TYPE_FAT12)){
		//TODO: fat12 support
	} else if(!strcmp(self->vfs->type, FILESYS_TYPE_FAT16)){
		//TODO: fat16 support		
	} else if(!strcmp(self->vfs->type, FILESYS_TYPE_FAT32)){
		do {
			currentSector = partitionOffset + metadata->firstDataSector + metadata->bpb.sectorsPerCluster * (currentCluster - 2);

			for(int i = 0; i < metadata->bpb.sectorsPerCluster; i++){
				buf = bcache_get(self->vfs->drive, currentSector);
				if(!buf) return NULL;

				directories = (fat_directory_t*)buf->data;

				for(int j = 0; j < metadata->bpb.bytesPerSector/sizeof(fat_directory_t); j++){
					if(directories[j].name[0] == 0xE5) continue;
					if(directories[j].name[0] == 0x00) break;

					if(directories[j].attrib & FAT_ATTR_LFN){
						isLFN = true;
						lfn = (fat_lfn_t*)&directories[j];
						retrievedName = fat_parse_lfn(lfn);

						if(retrievedName && (lfn->ord == 0x1 || lfn->ord == 0x41)){							
							if(!strcmp(retrievedName, lookFor)){
								kmfree(retrievedName);
								foundByLFN = true;
							}
						}

						continue;
					}

					if(isLFN & !foundByLFN){
						isLFN = false;
						continue;
					} else if(foundByLFN){
						isLFN = false;
						foundByLFN = false;
						goto fat32_found;
					}

					retrievedName = fat_parse_sfn(&directories[j]);
					if(!strcmp(retrievedName, lookFor)){
						kmfree(retrievedName);
						goto fat32_found;
					}

					continue;

fat32_found:
					vnode_t* vnode = vnode_new(self->vfs, &fatVnodeOps);

					fat_vnode_data_t* fsData = kmalloc(sizeof(fat_vnode_data_t));
					fsData->firstCluster = directories[j].clusterLow + (directories[j].clusterHigh << 16);

					vnode->fsData = fsData;

					memcpy(vnode->filename, fullname, 256);
					if(directories[j].attrib & FAT_ATTR_DIRECTORY){
						vnode->type = V_DIR;
					} else {
						vnode->type = V_REG;
					}

					vnode->size = directories[j].size;

					//TODO: vnode access times
					
					bcache_release(buf);
					
					return vnode;
				}

				bcache_release(buf);

				currentSector++;
			}
		} while(currentCluster = fat32_next_cluster(self->vfs, metadata, currentCluster));
	} else {
		klog("Filesystem not FAT!\n", KLOG_FAILED, "FAT");
		kerrno = EINVAL;
		return NULL;
	}

	kerrno = ENOENT;
	return NULL;
}

ssize_t fat_read(vnode_t* self, size_t size, void* buf, size_t offset){	
	const size_t partitionOffset = self->vfs->drive->partitions[self->vfs->partition].startLBA;

	fat_metadata_t* metadata = (fat_metadata_t*)self->vfs->context;
	fat_vnode_data_t* fsData = (fat_vnode_data_t*)self->fsData;

	size_t currentCluster = fsData->firstCluster;
    size_t currentSector = partitionOffset + metadata->firstDataSector + metadata->bpb.sectorsPerCluster * (currentCluster - 2);

	size_t currentPosition = 0, bytesRead = 0;

	if(offset >= self->size) return 0;

	uint8_t sectBuf[metadata->bpb.bytesPerSector];

	while(bytesRead < size && currentCluster && bytesRead != self->size){		
        for(size_t i = 0; i < metadata->bpb.sectorsPerCluster; i++){
			self->vfs->drive->read(self->vfs->drive, currentSector+i, 1, sectBuf);

			for(size_t k = 0; k < metadata->bpb.bytesPerSector; k++){
                if(currentPosition < offset){
                   currentPosition++;
                   continue;
                }
                ((uint8_t*)buf)[bytesRead] = sectBuf[k];
                offset++;
                bytesRead++; currentPosition++;
                if(bytesRead >= self || bytesRead >= self->size || bytesRead > size) break;
            }
		}
		if(!strcmp(self->vfs->type, FILESYS_TYPE_FAT12)){
			//TODO: fat12 support
		} else if(!strcmp(self->vfs->type, FILESYS_TYPE_FAT16)){
			//TODO: fat16 support
		} else if(!strcmp(self->vfs->type, FILESYS_TYPE_FAT32)){
			currentCluster = fat32_next_cluster(self->vfs, metadata, currentCluster);
		} else {
			klog("Filesystem not FAT!\n", KLOG_FAILED, "FAT");
			kerrno = EINVAL;
			return -EINVAL;
		}
		currentSector = partitionOffset + metadata->firstDataSector + metadata->bpb.sectorsPerCluster * (currentCluster - 2);
	}

	return bytesRead;
}

ssize_t fat_readdir(vnode_t* self, size_t count, void* buf, size_t offset){
	if(self->type != V_DIR){
		kerrno = ENOTDIR;
		return -ENOTDIR;
	}

	kprintf("FAT_READDIR CALLED!");
	while(1);
	
	return 0;
}
