/*  
*   File: fat.c
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause
#ifndef FAT_H
#define FAT_H

#include <sys/fal/fal.h>
#include <sys/dal/dal.h>

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
}__attribute__((packed)) fat_bpb_t;

typedef struct {
    fat_bpb_t bpb;
    uint8_t driveNumber;
    uint8_t NTFlags;
    uint8_t signature;
    uint32_t volSerial;
    char volLabel[11];
    char systemID[8];
}__attribute__((packed)) fat12_16_ebpb_t;

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
}__attribute__((packed)) fat32_ebpb_t;

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
}__attribute__((packed)) fat_directory_t;

typedef struct {
    uint8_t ord;
    uint16_t name1[5];
    uint8_t attr;
    uint8_t type;
    uint8_t checksum;
    uint16_t name2[6];
    uint16_t clusterLow;
    uint16_t name3[2];
}__attribute__((packed)) fat_lfn_t;

bool fat_probe(drive_t* drive, size_t partition);
bool fat_attach(drive_t* drive, size_t partition);

#endif //FAT_H