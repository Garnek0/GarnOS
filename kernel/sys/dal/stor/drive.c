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

int drive_add(drive_t* drive){
    drive->partitionCount = 0;

    drive->isSystemDrive = false; //assume this is not a system drive

	drive->driveid = _drive_gen_driveid();

	if(!drivesHead){
		drivesHead = drivesTail = drive;
		drivesHead->next = NULL;
	} else {
		drivesTail->next = drive;
		drivesTail = drive;
	}

    lock(drive->lock, {
        klog("Found Drive \"%s\".\n", KLOG_OK, "DAL", drive->name);
	
        //search for partitions

        if(drive->type == DRIVE_TYPE_OPTICAL){
            //partitions are ignored for optical media
            drive->partitionCount = 1;
            drive->partitions[0].attribs = 0;
            drive->partitions[0].startLBA = 0;
            drive->partitions[0].endLBA = 0;
            drive->partitions[0].size = 0;
            drive->partitions[0]._valid = true;
        } else if(gpt_validate_drive(drive)){
            if(!gpt_initialise_drive(drive)) klog("Drive \"%s\" has a corrupt GPT Header or Table!\n", KLOG_FAILED, "DAL", drive->name);
        } else {
            klog("Drive \"%s\" not partitioned or partition table unsupported!\n", KLOG_WARNING, "DAL", drive->name);
        }
        //TODO: Add MBR Partition Table support
		
        //search for filesystems

        for(size_t i = 0; i < drive->partitionCount; i++){
			device_t* fsPdev = kmalloc(sizeof(device_t));
			memset(fsPdev, 0, sizeof(device_t));

			fsPdev->name = "Filesystem Pseudodevice";
			fsPdev->bus = DEVICE_BUS_NONE;
			fsPdev->type = DEVICE_TYPE_FS_PSEUDODEVICE;
			
			device_id_initialise(fsPdev);
			device_id_add(fsPdev, DEVICE_CREATE_ID_FS_PDEV);

			fs_pdev_data_t* pdevData = kmalloc(sizeof(fs_pdev_data_t));
			pdevData->drive = drive;
			pdevData->partitionIndex = i;

			fsPdev->privateData = (void*)pdevData;

			device_add(fsPdev);
        }
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
				return 0;
			}
			prev = i;
		}
    });
}

drive_t* drive_get_list(){
    return drivesHead;
}
