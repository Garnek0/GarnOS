/*  
*   File: fat.c
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause
#ifndef FAT_H
#define FAT_H

#include <garn/fal/vnode.h>
#include <garn/fal/vfs.h>
#include <garn/dal/dal.h>
#include <garn/dal/bcache.h>

#define FAT_ATTR_READ_ONLY 0x01
#define FAT_ATTR_HIDDEN 0x02
#define FAT_ATTR_SYSTEM 0x04
#define FAT_ATTR_VOLUME_ID 0x08
#define FAT_ATTR_DIRECTORY 0x10
#define FAT_ATTR_ARCHIVE 0x20
#define FAT_ATTR_LFN (FAT_ATTR_READ_ONLY | FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM | FAT_ATTR_VOLUME_ID) 

typedef struct {
    uint8_t jump[3];
    char OEMID[8];
    uint16_t bytesPerSector;
    uint8_t sectsPerCluster;
    uint16_t reservedSectors;
    uint8_t FATCount;
    uint16_t rootDirEntryCount;
    uint16_t totalSects16;
    uint8_t mediaDescriptorType;
    uint16_t sectsPerFAT;
    uint16_t sectsPerTrack;
    uint16_t headCount;
    uint32_t hiddenSectorCount;
    uint32_t totalSects32;
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
fat_bpb_t;

typedef struct {
    fat_bpb_t bpb;
    uint8_t driveNumber;
    uint8_t NTFlags;
    uint8_t signature;
    uint32_t volSerial;
    char volLabel[11];
    char systemID[8];
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
fat12_16_ebpb_t;

typedef struct {
    fat_bpb_t bpb;
    uint32_t sectsPerFAT;
    uint16_t flags;
    uint16_t fatVer;
    uint32_t rootDirCluster;
    uint16_t FSInfoSect;
    uint16_t backupMBRSect;
    uint8_t reserved[12];
    uint8_t driveNumber;
    uint8_t NTFlags;
    uint8_t signature;
    uint32_t volSerial;
    char volLabel[11];
    char systemID[8];
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
fat32_ebpb_t;

typedef struct {
    void* ebpb;
    size_t totalSectors;
    size_t FATSize;
    size_t rootDirSectors;
    size_t firstDataSector;
    size_t firstRootDirSector;
    size_t firstFATSector;
    size_t dataSectors;
    size_t clusterCount;
    size_t clusterSize;
    size_t sectorsPerCluster;
    size_t bytesPerSector;
} fat_context_t;

typedef struct {
    char name[11];
    uint8_t attr;
    uint8_t reserved;
    uint8_t crtTimeTenth;
    uint16_t crtTime;
    uint16_t crtDate;
    uint16_t lstAccDate;
    uint16_t clusterHigh;
    uint16_t wrtTime;
    uint16_t wrtDate;
    uint16_t clusterLow;
    uint32_t size;
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
fat_directory_t;

typedef struct {
    uint8_t ord;
    uint16_t name1[5];
    uint8_t attr;
    uint8_t type;
    uint8_t checksum;
    uint16_t name2[6];
    uint16_t clusterLow;
    uint16_t name3[2];
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
fat_lfn_t;

typedef struct {
    size_t startCluster;
} fat_vnode_fs_data_t;

size_t fat12_next_cluster(vfs_t* filesys, fat_context_t* context, size_t cluster);
size_t fat16_next_cluster(vfs_t* filesys, fat_context_t* context, size_t cluster);
size_t fat32_next_cluster(vfs_t* filesys, fat_context_t* context, size_t cluster);
char* fat_parse_sfn(fat_directory_t* sd1);
bool fat_parse_and_compare_sfn(fat_directory_t* sd1, char* s2);
char* fat_parse_lfn(fat_lfn_t* lfn);

bool fat_parse_and_compare_lfn(fat_lfn_t* lfn, char* s2);

vnode_t* fat_open(vfs_t* self, char* path, int flags, int mode);
ssize_t fat_read(vfs_t* self, vnode_t* file, size_t size, void* buf, size_t offset);
ssize_t fat_write(vfs_t* self, vnode_t* file, size_t size, void* buf, size_t offset);
int fat_close(vfs_t* self, vnode_t* file);
int fat_mkdir(vfs_t* self, char* path);
int fat_rmdir(vfs_t* self, char* path);

bool fat_probe(drive_t* drive, size_t partition);
bool fat_attach(drive_t* drive, size_t partition);

#endif //FAT_H
