#include <sys/dal/dal-internals.h>
#include <garn/arch.h>
#include <garn/kstdio.h>
#include <garn/mm.h>
#include <garn/dal/dal.h>
#include <garn/config.h>
#include <uacpi/uacpi.h>
#include <uacpi/namespace.h>
#include <uacpi/utilities.h>
#include <uacpi/resources.h>
#include <uacpi/acpi.h>
#include <uacpi/tables.h>

static uacpi_ns_iteration_decision acpi_init_device(void* ctx, uacpi_namespace_node* node){
	uacpi_namespace_node_info* info;

	uacpi_status ret = uacpi_get_namespace_node_info(node, &info);
	if(uacpi_unlikely_error(ret)){
		const char* path = uacpi_namespace_node_generate_absolute_path(node);
		klog("Unable to retrieve node %s information: %s\n", KLOG_FAILED, "acpidev", path, uacpi_status_to_string(ret));
		uacpi_free_absolute_path(path);
		return UACPI_NS_ITERATION_DECISION_CONTINUE;
	}

	if(info->type != UACPI_OBJECT_DEVICE){
		uacpi_free_namespace_node_info(info);
		return UACPI_NS_ITERATION_DECISION_CONTINUE;
	}

	device_t* device = kmalloc(sizeof(device_t));
    device->bus = DEVICE_BUS_ACPI;
    device->data = NULL;
    device->name = "ACPI Device";
    device->node = NULL;
    device->type = DEVICE_TYPE_SYSTEM_DEVICE;
	device->data = (void*)info;
	device_id_initialise(device);

	if(info->flags & UACPI_NS_NODE_INFO_HAS_HID){
		char* hid = strdup(info->hid.value);
		device_id_add(device, DEVICE_CREATE_ID_ACPI(hid));
	}

	if(info->flags & UACPI_NS_NODE_INFO_HAS_CID){
		for(int i = 0; i < info->cid.num_ids; i++){
			char* cid = strdup(info->cid.ids[i].value);
			device_id_add(device, DEVICE_CREATE_ID_ACPI(cid));
		}
	}

	device_add(device);

	return UACPI_NS_ITERATION_DECISION_CONTINUE;
}

void acpidev_detect(){

#ifdef CONFIG_INCLUDE_i8042_DRIVER

    //Detect i8042 PS/2 Controller

	// The i8042 PS/2 Controller can easily be detected by testing a bit in the ACPI FADT

	uacpi_table FADTTable;
    uacpi_table_find_by_signature(ACPI_FADT_SIGNATURE, &FADTTable);
    struct acpi_fadt* FADT = (struct acpi_fadt*)FADTTable.virt_addr;

    if(FADT!=NULL && (FADT->iapc_boot_arch & (1 << 1))){
        goto i8042_found;
    } else {
        arch_inb(0x60);

        uint8_t res;
        arch_outb(0x64, 0xAA);
        size_t timeout = 100000;

        while(timeout--) if(arch_inb(0x64) & 1) break;
        if(!timeout) goto i8042_not_found;
        res = arch_inb(0x60);

        if(res == 0x55) goto i8042_found;
    }

i8042_found:
    device_t* ps2controller = kmalloc(sizeof(device_t));
    ps2controller->bus = DEVICE_BUS_NONE;
    ps2controller->data = NULL;
    ps2controller->name = "i8042 PS/2 Controller";
    ps2controller->node = NULL;
    ps2controller->type = DEVICE_TYPE_INPUT_CONTROLLER;
	device_id_initialise(ps2controller);
	device_id_add(ps2controller, DEVICE_CREATE_ID_PS2);
    device_add(ps2controller);

i8042_not_found:
    ;

#endif //CONFIG_INCLUDE_i8042_DRIVER

#ifdef CONFIG_INCLUDE_PIT_DRIVER

	//TODO: Use ACPI for this

    device_t* pitDev = kmalloc(sizeof(device_t));
    pitDev->bus = DEVICE_BUS_NONE;
    pitDev->data = NULL;
    pitDev->name = "Programmable Interval Timer";
    pitDev->node = NULL;
    pitDev->type = DEVICE_TYPE_SYSTEM_DEVICE;
	device_id_initialise(pitDev);
	device_id_add(pitDev, DEVICE_CREATE_ID_TIMER(DEVICE_ID_TIMER_PIT));
    device_add(pitDev);

#endif //CONFIG_INCLUDE_PIT_DRIVER
	
	uacpi_namespace_for_each_node_depth_first(uacpi_namespace_root(), acpi_init_device, UACPI_NULL);
}
