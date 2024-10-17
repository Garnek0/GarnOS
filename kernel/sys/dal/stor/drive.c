#include "garn/dal/dal.h"
#include "garn/dal/device-types.h"
#include <sys/dal/dal-internals.h>
#include <garn/mm.h>
#include <garn/panic.h>
#include <garn/fal/vnode.h>
#include <garn/fal/vfs.h>

drive_t* drivesHead;
drive_t* drivesTail;
size_t driveCount;

static size_t _drive_gen_driveid(){
	static size_t driveid = 0;
	return driveid++;
}

// Generate dynamic drive name
static char* _drive_gen_name(int type){

	// Keep track of how many devices of each type are
	// installed. We dont reuse letters and numbers if, for
	// example, a removable device is unplugged.
	static int sdCount = 0;
	static int srCount = 0;
	static int fdCount = 0;
	static int nvmeCount = 0;

	if(type == DRIVE_TYPE_OPTICAL || type == DRIVE_TYPE_FLOPPY || type == DRIVE_TYPE_NVME){
		// Set drvCount to hold the current drive type count
		int drvCount;
		if(type == DRIVE_TYPE_OPTICAL) drvCount = srCount;
		else if(type == DRIVE_TYPE_FLOPPY) drvCount = fdCount;
		else drvCount = nvmeCount;

		// Count the digits of the current device number.
		int drvCountCopy = drvCount;
		int dCount = 0;
		do {
			dCount++;
			drvCountCopy/=10;
		} while (drvCountCopy != 0);

		char* name;

		// Allocate the name string (dCount + 2 (the "sr" or "fd" characters) + 1 (the NULL terminator))
		if(type == DRIVE_TYPE_OPTICAL || type == DRIVE_TYPE_FLOPPY) name = kmalloc(3 + dCount);
		// Allocate the name string (dCount + 4 (the "nvme" characters) + 1 (the NULL terminator))
		else name = kmalloc(5 + dCount);

		// Set the drive name prefix and append the number.
		drvCountCopy = drvCount;

		if(type == DRIVE_TYPE_OPTICAL) {name[0] = 's'; name[1] = 'r';}
		else if(type == DRIVE_TYPE_FLOPPY) {name[0] = 'f'; name[1] = 'd';}
		else  {name[0] = 'n'; name[1] = 'v'; name[2] = 'm'; name[3] = 'e';}

		// Set 'i' and i's minimum value according to the drive type and add the NULL terminator
		int i, mni;
		if(type == DRIVE_TYPE_OPTICAL || type == DRIVE_TYPE_FLOPPY){
			i = dCount + 1;
			mni = 1;
			name[2+dCount] = 0;
		} else {
			i = dCount + 3;
			mni = 3;
			name[4+dCount] = 0;
		}

		// Fill in the name string's drive number
		for(; i > mni; i--){
			name[i] = (char)(0x30 + drvCountCopy%10);
			drvCountCopy/=10;
		}

		// Increase current drive type count.
		if(type == DRIVE_TYPE_OPTICAL) srCount++;
		else if(type == DRIVE_TYPE_FLOPPY) fdCount++;
		else nvmeCount++;

		return name;
	} else if(type == DRIVE_TYPE_GENERIC){
		// Count the numbers of letters needed (with 0-based indexing)
		int sdCountCopy = sdCount+1;
		int letterCount = 0;
		do {
			sdCountCopy--;
			letterCount++;
			sdCountCopy/=26;
		} while (sdCountCopy != 0);	
		
		sdCountCopy = sdCount+1;

		// (Same as the optical drive implementation, except
		// letters are used instead of numbers and the counting
		// system is a little different)

		char* name = kmalloc(3 + letterCount);

		name[0] = 's'; name[1] = 'd';
		for(int i = 1+letterCount; i > 1; i--){
			sdCountCopy--;
			name[i] = 97 + sdCountCopy%26;
			sdCountCopy/=26;
		}

		name[2+letterCount] = 0;

		sdCount++;

		return name;
	}

	return NULL;
}

int drive_add(drive_t* drive){
    drive->partitionCount = 0;

    drive->isSystemDrive = false; //assume this is not the system drive

	drive->driveid = _drive_gen_driveid();

	if(!drivesHead){
		drivesHead = drivesTail = drive;
		drivesHead->next = NULL;
	} else {
		drivesTail->next = drive;
		drivesTail = drive;
	}

    lock(drive->lock, {
		char* driveName = _drive_gen_name(drive->type);

        klog("Found Drive \"%s\" (%s)\n", KLOG_OK, "DAL", drive->name, driveName);

		device_t* driveDev = kmalloc(sizeof(device_t));
		memset(driveDev, 0, sizeof(device_t));
		driveDev->name = driveName;
		driveDev->bus = DEVICE_BUS_NONE;
		driveDev->type = DEVICE_TYPE_STORAGE_MEDIUM;
		driveDev->category = DEVICE_CAT_BLOCK;
		// Very barebones major number determination based on the drive type.
		// Although doing major device numbers like this is not the best
		// idea, device numbers as a whole are barely used anymore nowadays.
		if(drive->type == DRIVE_TYPE_FLOPPY) driveDev->major = 2;
		else if(drive->type == DRIVE_TYPE_GENERIC) driveDev->major = 8;
		else if(drive->type == DRIVE_TYPE_OPTICAL) driveDev->major = 11;
		else if(drive->type == DRIVE_TYPE_NVME) driveDev->major = 242;
		// Device minor number is 0 for the whole disk.
		driveDev->minor = 0;
		driveDev->blockSize = 512;
		driveDev->devOps = NULL; //TODO: This

        // Search for partitions

        if(drive->type == DRIVE_TYPE_OPTICAL){
            // Partitions are ignored for optical media
            drive->partitionCount = 1;
            drive->partitions[0].attribs = 0;
            drive->partitions[0].startLBA = 0;
            drive->partitions[0].endLBA = 0;
            drive->partitions[0].size = 0;
            drive->partitions[0]._valid = true;

			// Since there are no partitions, set up the drive device
			// as a storage medium for the drivers
			
			device_id_initialise(driveDev);
			device_id_add(driveDev, DEVICE_CREATE_ID_SM);

			fs_pdev_data_t* pdevData = kmalloc(sizeof(fs_pdev_data_t));
			pdevData->drive = drive;
			pdevData->partitionIndex = 0;

			driveDev->privateData = (void*)pdevData;
        } else if(gpt_validate_drive(drive)){
            if(!gpt_initialise_drive(drive)) klog("Drive \"%s\" has a corrupt GPT Header or Table!\n", KLOG_FAILED, "DAL", drive->name);
        } else {
            klog("Drive \"%s\" not partitioned or partition table unsupported!\n", KLOG_WARNING, "DAL", drive->name);
        }
        //TODO: Add MBR Partition Table support
		
        // Initialise partition devices

        for(size_t i = 0; i < drive->partitionCount; i++){
			if(drive->type == DRIVE_TYPE_OPTICAL) break;

			device_t* fsPdev = kmalloc(sizeof(device_t));
			memset(fsPdev, 0, sizeof(device_t));

			char* partName;

			// Generate partition name. This algorithm is very similar to the one
			// used in _drive_gen_name.
			int iCopy = i+1;
			int dCount = 0;
			do {
				dCount++;
				iCopy/=10;
			} while (iCopy != 0);

			if(fsPdev->type == DRIVE_TYPE_FLOPPY || fsPdev->type == DRIVE_TYPE_NVME){
				partName = kmalloc(strlen(driveDev->name)+1+dCount+1);
				partName[strlen(driveDev->name)] = 'p';
			} else {
				partName = kmalloc(strlen(driveDev->name)+dCount+1);
			}

			memcpy(partName, driveDev->name, strlen(driveDev->name));
			
			iCopy = i+1;

			if(fsPdev->type == DRIVE_TYPE_FLOPPY || fsPdev->type == DRIVE_TYPE_NVME){
				for(uint32_t i = strlen(driveDev->name) + dCount; i > strlen(driveDev->name); i--){
					partName[i] = (char)(0x30 + iCopy%10);
					iCopy/=10;
				}
			} else {
				for(uint32_t i = strlen(driveDev->name)-1 + dCount; i > strlen(driveDev->name)-1; i--){
					partName[i] = (char)(0x30 + iCopy%10);
					iCopy/=10;
				}
			}

			partName[strlen(driveDev->name)+1+dCount] = 0;

			if(fsPdev->type == DRIVE_TYPE_FLOPPY || fsPdev->type == DRIVE_TYPE_NVME) partName[strlen(driveDev->name)+1+dCount] = 0;
			else partName[strlen(driveDev->name)+dCount] = 0;

			fsPdev->name = partName;
			fsPdev->bus = DEVICE_BUS_NONE;
			fsPdev->type = DEVICE_TYPE_STORAGE_MEDIUM;
			fsPdev->category = DEVICE_CAT_BLOCK;
			if(drive->type == DRIVE_TYPE_FLOPPY) fsPdev->major = 2;
			else if(drive->type == DRIVE_TYPE_GENERIC) fsPdev->major = 8;
			else if(drive->type == DRIVE_TYPE_OPTICAL) fsPdev->major = 11;
			else if(drive->type == DRIVE_TYPE_NVME) fsPdev->major = 242;
			// Device minor number is i for partition[i], where i > 0;
			// (i starts at 0 here, so we need to add 1)
			fsPdev->minor = i+1;
			fsPdev->blockSize = 512;
			fsPdev->devOps = NULL; //TODO: This

			device_id_initialise(fsPdev);
			device_id_add(fsPdev, DEVICE_CREATE_ID_SM);

			fs_pdev_data_t* pdevData = kmalloc(sizeof(fs_pdev_data_t));
			pdevData->drive = drive;
			pdevData->partitionIndex = i;

			fsPdev->privateData = (void*)pdevData;

			device_add(fsPdev);
        }

		device_add(driveDev);
    });

    return 0;
}

void drive_remove(drive_t* drive){
    lock(drive->lock, {
		drive_t* prev = NULL;
		for(drive_t* i = drivesHead; i; i = i->next){
			if(i == drive){
				if(prev){
					lock(prev->lock, {
						prev->next = i->next;
					});
				} else {
					drivesHead = drivesHead->next;
				}
				kmfree(i);
				return;
			}
			prev = i;
		}
    });
}

drive_t* drive_get_list(){
    return drivesHead;
}
