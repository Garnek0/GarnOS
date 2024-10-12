#ifndef FAT_H
#define FAT_H

#include <garn/types.h>
#include <garn/fal/vnode.h>
#include <garn/fal/vfs.h>
#include <garn/dal/dal.h>
#include <garn/dal/bcache.h>

#define FAT_ATTR_READ_ONLY 1
#define FAT_ATTR_HIDDEN (1 << 1)
#define FAT_ATTR_SYSTEM (1 << 2)
#define FAT_ATTR_VOL_ID (1 << 3)
#define FAT_ATTR_DIRECTORY (1 << 4)
#define FAT_ATTR_ARCHIVE (1 << 5)
#define FAT_ATTR_LFN (FAT_ATTR_READ_ONLY | FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM | FAT_ATTR_VOL_ID) 

typedef struct {
	uint8_t jump[3];
	char OEMID[8];
	uint16_t bytesPerSector;
	uint8_t sectorsPerCluster;
	uint16_t reservedSectorCount;
	uint8_t FATCount;
	uint16_t rootEntryCount;
	uint16_t totalSectors16;
	uint8_t media;
	uint16_t FATSize16;
	uint16_t sectorsPerTrack;
	uint16_t headCount;
	uint32_t hiddenSectorCount;
	uint32_t totalSectors32;
}__attribute__((packed)) fat_common_bpb_t;

typedef struct {
	fat_common_bpb_t commonBPB;
	uint8_t drvNumber;
	uint8_t reserved;
	uint8_t bootSig;
	uint32_t volID;
	char volLabel[11];
	char FilesysType[8];	
}__attribute__((packed)) fat12_bpb_t;

typedef fat12_bpb_t fat16_bpb_t;

typedef struct {
	fat_common_bpb_t commonBPB;
	uint32_t FATSize32;
	uint16_t ExtFlags;
	uint16_t FatVersion;
	uint32_t rootCluster;
	uint16_t FSInfoSector;
	uint16_t backupBootSector;
	uint8_t reserved0[12];
	uint8_t drvNumber;
	uint8_t reserved1;
	uint8_t bootSig;
	uint32_t volID;
	char volLabel[11];
	char FilesysType[8];
}__attribute__((packed)) fat32_bpb_t;

typedef struct {
	char name[11];
	uint8_t attrib;
	uint8_t reserved;
	uint8_t unused;
	uint16_t creationTime;
	uint16_t creationDate;
	uint16_t lastAccessedDate;
	uint16_t clusterHigh;
	uint16_t lastModificationTime;
	uint16_t lastModificationDate;
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

typedef struct {
	fat_common_bpb_t bpb;
	size_t sectorCount;
	uint32_t FATSize;
	uint32_t rootDirSectors;
	uint32_t firstDataSector;
	uint32_t firstFATSector;
	size_t dataSectorCount;
	size_t clusterCount;
} fat_metadata_t;

typedef struct {
	uint32_t firstCluster;
} fat_vnode_data_t;

statfs_t fat_statfs(vfs_t* self);

vnode_t* fat_lookup(vnode_t* self, const char* name);
ssize_t fat_read(vnode_t* self, size_t size, void* buf, size_t offset);
ssize_t fat_readdir(vnode_t* self, size_t count, void* buf, size_t offset);

static const vnode_operations_t fatVnodeOps = {
	.vn_read = fat_read,
	.vn_write = NULL,
	.vn_ioctl = NULL,
	.vn_inactive = NULL,
	.vn_lookup = fat_lookup,
	.vn_mkdir = NULL,
	.vn_rmdir = NULL,
	.vn_readdir = fat_readdir,
};

#endif //FAT_H
