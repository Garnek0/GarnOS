#include "fat.h"

#include <garn/module.h>
#include <garn/types.h>
#include <garn/dal/dal.h>
#include <garn/mm.h>
#include <garn/kstdio.h>
#include <garn/time.h>

#define PROBE_OEMID_COUNT 15

static vfs_operations_t fatVFSOps = {
	.vfs_statfs = fat_statfs
};

const char* probeOEMIDs[PROBE_OEMID_COUNT] = {
    "GARNFAT ", //Garn Kernel custom OEMID (I dont really know when/if this will be used)
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
    "mkdosfs ",
    "mkfs.fat"
};

bool probe(device_t* device){
	fat_common_bpb_t* bpb;
	fs_pdev_data_t* pdevData = (fs_pdev_data_t*)device->privateData;

	if(pdevData->drive->type == DRIVE_TYPE_OPTICAL) return false;

    bcache_buf_t* buf = bcache_get(pdevData->drive, pdevData->drive->partitions[pdevData->partitionIndex].startLBA);
	if(!buf) return false;

    bpb = (fat_common_bpb_t*)buf->data;

	if(bpb->bytesPerSector == 0){
		// ExFAT not supported by this driver
		bcache_release(buf);
		return false;
	}

    if(bpb->jump[0] != 0xEB && bpb->jump[2] != 0x90) {
        bcache_release(buf);
        return false;
    }

    for(int i = 0; i < PROBE_OEMID_COUNT; i++){
        if(!strncmp((const char*)bpb->OEMID, probeOEMIDs[i], 8)){
            //This is surely a FAT filesystem
            bcache_release(buf);
            return true;
        }
    }

    bcache_release(buf);
    return false;	
}

bool attach(device_t* device){
	fs_pdev_data_t* pdevData = (fs_pdev_data_t*)device->privateData;
	drive_t* drive = pdevData->drive;
	size_t partition = pdevData->partitionIndex; 

	// Probing done, remove the filesystem pseudodevice now as it is no longer needed
	device_remove(device);

	// Get sector 0 from the partition
	bcache_buf_t* buf = bcache_get(drive, drive->partitions[partition].startLBA);
	if(!buf) return false;

	fat_metadata_t* metadata = kmalloc(sizeof(fat_metadata_t)); 
	fat_common_bpb_t* bpb;
	fat12_bpb_t* fat12bpb;
	fat16_bpb_t* fat16bpb;
	fat32_bpb_t* fat32bpb;

	// Allocate the FAT VFS struct

	vfs_t* fatVFS = kmalloc(sizeof(vfs_t));
	memset(fatVFS, 0, sizeof(vfs_t));

	// Calculate some stuff which will be needed later

	bpb = (fat_common_bpb_t*)buf->data;
	fat12bpb = (fat12_bpb_t*)bpb;
	fat16bpb = (fat16_bpb_t*)bpb;
	fat32bpb = (fat32_bpb_t*)bpb;

	metadata->bpb = *bpb;
	metadata->sectorCount = (bpb->totalSectors16 == 0) ? bpb->totalSectors32 : bpb->totalSectors16;
	metadata->FATSize = (bpb->FATSize16 == 0) ? fat32bpb->FATSize32 : bpb->FATSize16;
	metadata->rootDirSectors = ((bpb->rootEntryCount * 32) + (bpb->bytesPerSector - 1)) / bpb->bytesPerSector;
	metadata->firstDataSector = bpb->reservedSectorCount + (bpb->FATCount * metadata->FATSize) + metadata->rootDirSectors;
	metadata->firstFATSector = bpb->reservedSectorCount;
	metadata->dataSectorCount = metadata->sectorCount - (bpb->reservedSectorCount + (bpb->FATCount * metadata->FATSize) + metadata->rootDirSectors);
	metadata->clusterCount = metadata->dataSectorCount / bpb->sectorsPerCluster;	

	fatVFS->context = (void*)metadata;

	fat_vnode_data_t* rootDirData = kmalloc(sizeof(fat_vnode_data_t));
	memset(rootDirData, 0, sizeof(fat_vnode_data_t));

	if(metadata->clusterCount < 4085){
		klog("Found FAT12 Filesystem.\n", KLOG_OK, "FAT");
		memcpy(fatVFS->type, FILESYS_TYPE_FAT12, strlen(FILESYS_TYPE_FAT12)+1);
		
		// Set volume label
		
		if(fat12bpb->bootSig == 0x29){
			memcpy(fatVFS->name, fat12bpb->volLabel, 11);
		} else {
			memcpy(fatVFS->name, "NO NAME   ", 11);
		}
 
	} else if(metadata->clusterCount < 65525){
		klog("Found FAT16 Filesystem.\n", KLOG_OK, "FAT");
		memcpy(fatVFS->type, FILESYS_TYPE_FAT16, strlen(FILESYS_TYPE_FAT16)+1);
	
		// Set volume label
		
		if(fat16bpb->bootSig == 0x29){
			memcpy(fatVFS->name, fat16bpb->volLabel, 11);
		} else {
			memcpy(fatVFS->name, "NO NAME   ", 11);
		}

	} else {
		klog("Found FAT32 Filesystem.\n", KLOG_OK, "FAT");
		memcpy(fatVFS->type, FILESYS_TYPE_FAT32, strlen(FILESYS_TYPE_FAT32)+1);
	
		// Set volume label
		
		if(fat32bpb->bootSig == 0x29){
			memcpy(fatVFS->name, fat32bpb->volLabel, 11);
		} else {
			memcpy(fatVFS->name, "NO NAME   ", 11);
		}

		rootDirData->firstCluster = fat32bpb->rootCluster;
	}

	fatVFS->fsops = &fatVFSOps;

	fatVFS->drive = drive;
	fatVFS->partition = partition;

	fatVFS->rootVnode = vnode_new(fatVFS, &fatVnodeOps);

	fatVFS->rootVnode->type = V_DIR;
	fatVFS->rootVnode->size = metadata->rootDirSectors * bpb->bytesPerSector;
	fatVFS->rootVnode->fsData = (void*)rootDirData;

	fatVFS->rootVnode->lastAccessedTime = time_get64();
	fatVFS->rootVnode->lastModifiedTime = fatVFS->rootVnode->lastChangedTime = time_get64(); // No way to know these values	

	//TODO: Make this better
	vfs_mount(fatVFS, NULL, 0);

	bcache_release(buf);

	return true;

//deallocate_and_fail:
//	bcache_release(buf);
//	kmfree(metadata);
//	kmfree(fatVFS);
//	return false;
}

bool remove(device_t* device){
	kmfree(device->privateData);
	return true;
}

void init(){
	return;
}

void fini(){
	return;
}

statfs_t fat_statfs(vfs_t* self){
	fat_metadata_t* metadata = (fat_metadata_t*)self->context;
	statfs_t statfs;

	statfs.type = 0x4D44; // MSDOS_SUPER_MAGIC
	statfs.bsize = metadata->bpb.sectorsPerCluster * metadata->bpb.bytesPerSector;
	statfs.totalBlocks = metadata->clusterCount;
	statfs.freeBlocks = 0;
	statfs.files = 0;
	statfs.freeFiles = 0;
	statfs.fid = self->fid;
	statfs.maxNameLength = 256;
	statfs.mountFlags = self->mountFlags;

	bcache_buf_t* buf;

	if(!strcmp(self->type, FILESYS_TYPE_FAT12)){
		//TODO: FAT12 support
	} else if(!strcmp(self->type, FILESYS_TYPE_FAT16)){
		uint16_t* TableEntry;
		for(size_t i = 0; i < metadata->FATSize; i++){
			buf = bcache_get(self->drive, self->drive->partitions[self->partition].startLBA+i);
			TableEntry = (uint16_t*)buf->data;
			for(int j = 0; j < 256; j++){
				if(TableEntry[j] == 0) statfs.freeBlocks++;
				else if(TableEntry[j] == 0xFFFF) statfs.files++;
			}
			bcache_release(buf);
		}
	} else if(!strcmp(self->type, FILESYS_TYPE_FAT32)){
		uint32_t* TableEntry;
		for(size_t i = 0; i < metadata->FATSize; i++){
			buf = bcache_get(self->drive, self->drive->partitions[self->partition].startLBA+i);
			TableEntry = (uint32_t*)buf->data;
			for(int j = 0; j < 128; j++){
				if((TableEntry[j] & 0x0FFFFFFF) == 0) statfs.freeBlocks++;
				else if((TableEntry[j] & 0x0FFFFFFF) == 0xFFFFFFF) statfs.files++;
			}
			bcache_release(buf);
		}
	}

	statfs.availBlocks = statfs.freeBlocks;

	return statfs;
}

module_t metadata = {
	.name = "fatfs", // Name of the module.
	.init = init,
	.fini = fini
};

device_driver_t driver_metadata = {
	.probe = probe,
	.attach = attach,
	.remove = remove
};

device_id_t driver_ids[] = {
	DEVICE_CREATE_ID_FS_PDEV,
	DEVICE_ID_LIST_END
};

