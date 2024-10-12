#pragma once

#include <garn/types.h>
#include <garn/spinlock.h>
#include <garn/dal/device-types.h>
#include <garn/ds/list.h>

#define MAX_PARTITIONS 128

#define DRIVE_IF_UNDEFINED 0
#define DRIVE_IF_IDE 1
#define DRIVE_IF_AHCI 2

#define DRIVE_TYPE_UNDEFINED 0
#define DRIVE_TYPE_DISK 1
#define DRIVE_TYPE_OPTICAL 2
#define DRIVE_TYPE_FLOPPY 3

#define PART_ATTRIB_REQUIRED 1
#define PART_ATTRIB_OS (1 << 1)

struct _driver_node;
struct _device;

typedef struct {
	uint8_t class;
	uint8_t value8[4];
	uint16_t value16[4];
	uint32_t value32[4];
 	char* string[2];
} device_id_t;

typedef struct _device {
    char* name;
    uint16_t bus;
    uint16_t type;
    void* data; //Data of the device
    void* driverData; //Drivers can store their own data in this field
    list_t* idList;
    struct _driver_node* node;
} device_t;

typedef struct _device_driver {
    bool (*probe)(struct _device* device);
    bool (*attach)(struct _device* device);
    bool (*remove)(struct _device* device);
    void* data;
} device_driver_t;

typedef struct _driver_node {
    device_driver_t* driver;
    device_id_t* ids;
    bool loaded;
    char* path;
} driver_node_t;

typedef struct _partition {
    uint64_t startLBA;
    uint64_t endLBA;
    uint64_t attribs;
    bool isSystemPartition;
    size_t size;

    //automatically set by the DAL. should not be touched
    //by drivers.
    bool _valid;
} partition_t;

typedef struct _drive {
    char* name;
    int interface;
    bool isSystemDrive;
    uint8_t type;
    size_t size;
    size_t blockSize;

	size_t driveid;

    int (*read)(struct _drive* self, uint64_t startLBA, size_t blocks, void* buf);
    int (*write)(struct _drive* self, uint64_t startLBA, size_t blocks, void* buf);

    //automatically set by the DAL. should not be touched
    //by drivers.
    size_t partitionCount;
    partition_t partitions[MAX_PARTITIONS];

    spinlock_t lock;

    void* context;

	struct _drive* next;
} drive_t;

typedef struct _fs_pdev_data {
	drive_t* drive;
	size_t partitionIndex;
} fs_pdev_data_t;

//device

void device_add(device_t* device);
size_t device_get_device_count();
device_t* device_get_device(size_t i);
bool device_attach_to_driver(struct _driver_node* node);
int device_remove(device_t* device);
bool device_match_ids(device_id_t id1, device_id_t id2);
void device_id_initialise(device_t* device);
void device_id_add(device_t* device, device_id_t devid);

//driver

void device_driver_add(driver_node_t* driver);
bool device_driver_attach(device_t* device);
int device_driver_unregister_node(driver_node_t* node);
int device_driver_unregister(const char* path);
int device_driver_register(const char* path);
int device_driver_autoreg(const char* path);
size_t device_driver_get_driver_count();
device_driver_t* device_driver_get_driver(size_t i);

//drive

int drive_add(drive_t* drive);
void drive_remove(drive_t* drive);
drive_t* drive_get_list();
