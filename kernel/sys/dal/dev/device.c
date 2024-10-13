#include <sys/dal/dal-internals.h>
#include <garn/mm.h>
#include <garn/hw/pci.h>
#include <garn/spinlock.h>
#include <garn/ds/list.h>
#include <garn/kstdio.h>
#include <garn/dal/dal.h>
#include <exec/elf.h>

spinlock_t deviceManagerLock;

list_t* deviceList;
list_t* charDeviceList;

size_t deviceCount;

void device_init(){
    deviceList = list_create();
	charDeviceList = list_create();
}

void device_add(device_t* device){
    list_insert(deviceList, (void*)device);
    deviceCount++;
    device_driver_attach(device);
}

int device_remove(device_t* device){
    if(device->node && device->node->loaded && device->node->driver->remove){
        if(!device->node->driver->remove(device)){
            klog("Could not remove device \'%s\'! Driver returned false.\n", KLOG_FAILED, "DAL", device->name);
            return -1;
        }
    }
    if(list_remove(deviceList, device) != 0) return -1;
    deviceCount--;
    return 0;
}

size_t device_get_device_count(){
    return deviceCount;
}

device_t* device_get_device(size_t i){
    device_t* device = NULL;
    size_t count = 0;

    if(i >= deviceCount) i = deviceCount;

    lock(deviceManagerLock, {
        foreach(item, deviceList){
            device = (device_t*)item->value;
            if(count == i) break;
            count++;
        }
    });

    return device;
}

// id1 should always be the driver's devid.
bool device_match_ids(device_id_t id1, device_id_t id2){
	if(DEVICE_ID_CLASS(id1) != DEVICE_ID_CLASS(id2)) return false;
	switch(DEVICE_ID_CLASS(id1)){
		case DEVICE_ID_CLASS_FS_PDEV:
		case DEVICE_ID_CLASS_PS2:
		{
			return true;
			break;
		}
        case DEVICE_ID_CLASS_TIMER:
		{
            if(DEVICE_ID_TIMER_TYPE(id1) == DEVICE_ID_TIMER_TYPE(id2)) return true;
            break;
		}
        case DEVICE_ID_CLASS_BUS:
        {
            if(DEVICE_ID_BUS_TYPE(id1) == DEVICE_ID_BUS_TYPE(id2)) return true;
            break;
		}
        case DEVICE_ID_CLASS_PCI:
		{
            if((DEVICE_ID_PCI_VENDOR(id1) == DEVICE_ID_PCI_VENDOR(id2) || DEVICE_ID_PCI_VENDOR(id1) == DEVICE_ID_PCI_VENDOR_ANY) &&
            (DEVICE_ID_PCI_DEVICE(id1) == DEVICE_ID_PCI_DEVICE(id2) || DEVICE_ID_PCI_DEVICE(id1) == DEVICE_ID_PCI_DEVICE_ANY) &&
            DEVICE_ID_PCI_CLASS(id1) == DEVICE_ID_PCI_CLASS(id2) &&
        	DEVICE_ID_PCI_SUBCLASS(id1) == DEVICE_ID_PCI_SUBCLASS(id2) &&
            (DEVICE_ID_PCI_PROGIF(id1) == DEVICE_ID_PCI_PROGIF(id2) || DEVICE_ID_PCI_PROGIF(id1) == DEVICE_ID_PCI_PROGIF_ANY)) return true;
            break;
        }
		case DEVICE_ID_CLASS_ACPI:
		{
			if(DEVICE_ID_ACPI_ID(id1) && DEVICE_ID_ACPI_ID(id2) && !strcmp(DEVICE_ID_ACPI_ID(id1), DEVICE_ID_ACPI_ID(id2))) return true;
			break;
		}
        default:
			return false;
            break;
	}
	return false;
}

void device_id_initialise(device_t* device){
	device->idList = list_create();
}

void device_id_add(device_t* device, device_id_t devid){
	device_id_t* newDevID = kmalloc(sizeof(device_id_t));
	*newDevID = devid;

	list_insert(device->idList, (void*)newDevID);
}

bool device_attach_to_driver(driver_node_t* node){
    if(deviceCount == 0) return false;

    device_t* device;
    device_driver_t* driver;
    bool status;
    int i = 0;

    foreach(item, deviceList){
        i = 0;
        status = false;
        device = (device_t*)item->value;
        if(!device) continue;

        for(;; i++){
			if(node->ids[i].class == DEVICE_ID_CLASS_NONE) break;
			foreach(id, device->idList){
				status = device_match_ids(node->ids[i], *((device_id_t*)id->value));
				if(status){
					klog("Found Possible Driver for %s\n", KLOG_OK, "DAL", device->name);
					if(!node->loaded) elf_load_driver(node);
    	            break;
				}
			}
			if(status) break;
        }
        if(!status) continue;

        driver = (device_driver_t*)node->driver;

		if(!driver || !driver->probe){
            continue;
        }

        status = driver->probe(device);
        if(status){
            device->node = node;
            klog("Found Driver for %s\n", KLOG_OK, "DAL", device->name);
            status = driver->attach(device);
            if(status){
                releaseLock(&deviceManagerLock);
                return true;
            } else {
                klog("Failed to attach device %s to %s\n", KLOG_FAILED, "DAL", device->name, node->path);
                device->node = NULL;
            }
        }
    }

    return false;
}

void device_register_cdev(char_device_t* cdev){
	list_insert(charDeviceList, (void*)cdev);
}

list_t* device_get_cdev_list(){
	return charDeviceList;
}
