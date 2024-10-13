//Example module

#include <garn/module.h>
#include <garn/types.h>
#include <garn/dal/dal.h>

// COMMON

// Called when the module gets loaded
void init(){
	return;
}

// Called when the module is unloaded, or when the system is
// about to power off
void fini(){
	return;
}

// DRIVER

// Called by the OS when a new device is detected. The driver should respond by
// checking "device" and returning true if it is valid
// for this driver, or false otherwise.
//
// Invalid devices for a certain driver are rarely
// encountered due to the use of device IDs, but it is
// still good practice to properly implement this function.
bool probe(device_t* device){
	return false;
}

// Called if probe() succeeded for "device"
//
// This function should contain code for initialising
// the device and return true if everything went well, 
// or false otherwise.
//
// At this point, the driver should also set the name of the device, because it is
// not guaranteed the kernel will set it correctly.
bool attach(device_t* device){
	return false;
}

// Called when "device" is removed
bool remove(device_t* device){
	return false;
}

// Contains info about this module
// This struct MUST be named "metadata"
module_t metadata = {
	.name = "example", // Name of the module.
	// Pointers to the init() and fini() routines
	.init = init,
	.fini = fini
};

// Contains driver data.
// This struct MUST be named "driver_metadata"
//
// This struct is only required for drivers. If you dont
// define "driver_metadata", then this module will be treated
// as a regular module
device_driver_t driver_metadata = {
	// Pointers to driver-specific routines probe() etc ...
	.probe = probe,
	.attach = attach,
	.remove = remove
};

// Contains the driver's device IDs. If the module is not a
// driver, then this array MUST NOT be defined. If the module
// is a driver, then this array MUST be defined and the last element
// MUST be DEVICE_ID_LIST_END.
// This array MUST be named "driver_ids"
device_id_t driver_ids[] = {
	// Device IDs go here, for example:
	// DEVICE_CREATE_ID_FOO(DEVICE_ID_FOO_BAR),
	// DEVICE_CREATE_ID_FOO(DEVICE_ID_FOO_BAZ),
	DEVICE_ID_LIST_END
};

