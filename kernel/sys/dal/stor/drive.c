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
	static int hdCount = 0;
	static int srCount = 0;
	static int fdCount = 0;

	if(type == DRIVE_TYPE_OPTICAL){
		int srCountCopy = srCount;
		int dCount = 0;
		do {
			dCount++;
			srCountCopy/=10;
		} while (srCountCopy != 0);

		char* name = kmalloc(3 + dCount);

		srCountCopy = srCount;

		name[0] = 's'; name[1] = 'r';
		for(int i = 1 + dCount; i > 1; i--){
			name[i] = (char)(0x30 + srCountCopy%10);
			srCountCopy/=10;
		}
		name[2+dCount] = 0;

		srCount++;

		return name;
	} else if(type == DRIVE_TYPE_FLOPPY){
		int fdCountCopy = fdCount;
		int dCount = 0;
		do {
			dCount++;
			fdCountCopy/=10;
		} while (fdCountCopy != 0);

		char* name = kmalloc(3 + dCount);

		fdCountCopy = fdCount;

		name[0] = 'f'; name[1] = 'd';
		for(int i = 1 + dCount; i > 1; i--){
			name[i] = (char)(0x30 + fdCountCopy%10);
			fdCountCopy/=10;
		}
		name[2+dCount] = 0;

		fdCount++;

		return name;	
	} else {
		int letterCount = hdCount/26+1;

	  	int hdCountCopy = hdCount;

		char* name = kmalloc(3 + letterCount);

		name[0] = 'h'; name[1] = 'd';
		for(int i = 2; i < 2+letterCount; i++){
			name[i] = 97 + hdCountCopy%26;
			hdCountCopy/=26;
		}

		name[2+letterCount] = 0;

		hdCount++;

		return name;
	}
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
		driveDev->major = 3; //WARN: This is not always correct
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
		
        //search for filesystems

        for(size_t i = 0; i < drive->partitionCount; i++){
			if(drive->type == DRIVE_TYPE_OPTICAL) break;

			device_t* fsPdev = kmalloc(sizeof(device_t));
			memset(fsPdev, 0, sizeof(device_t));

			char* partName;

			// Generate partition name
			int iCopy = i+1;
			int dCount = 0;
			do {
				dCount++;
				iCopy/=10;
			} while (iCopy != 0);

			if(fsPdev->type == DRIVE_TYPE_FLOPPY){
				partName = kmalloc(strlen(driveDev->name)+1+dCount+1);
				partName[strlen(driveDev->name)] = 'p';
			} else {
				partName = kmalloc(strlen(driveDev->name)+dCount+1);
			}

			memcpy(partName, driveDev->name, strlen(driveDev->name));
			
			iCopy = i+1;

			if(fsPdev->type == DRIVE_TYPE_FLOPPY){
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

			if(fsPdev->type == DRIVE_TYPE_FLOPPY) partName[strlen(driveDev->name)+1+dCount] = 0;
			else partName[strlen(driveDev->name)+dCount] = 0;

			fsPdev->name = partName;
			fsPdev->bus = DEVICE_BUS_NONE;
			fsPdev->type = DEVICE_TYPE_STORAGE_MEDIUM;
			fsPdev->category = DEVICE_CAT_BLOCK;
			fsPdev->major = 3; //WARN: This is not always correct
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
