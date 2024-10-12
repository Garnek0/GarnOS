#pragma once

#include <garn/types.h>

#define DEVICE_BUS_UNDEFINED 0
#define DEVICE_BUS_NONE 1
#define DEVICE_BUS_PCI 2
#define DEVICE_BUS_ACPI 3

#define DEVICE_TYPE_UNDEFINED 0
#define DEVICE_TYPE_STORAGE_CONTROLLER 1
#define DEVICE_TYPE_NETWORK_CONTROLLER 2
#define DEVICE_TYPE_DISPLAY_CONTROLLER 3
#define DEVICE_TYPE_MULTIMEDIA_CONTROLLER 4
#define DEVICE_TYPE_MEMORY_CONTROLLER 5
#define DEVICE_TYPE_SERIAL_DEVICE 6
#define DEVICE_TYPE_PARALLEL_DEVICE 7
#define DEVICE_TYPE_GPIB 8
#define DEVICE_TYPE_COMMUNICATION_DEVICE 9
#define DEVICE_TYPE_MODEM 10
#define DEVICE_TYPE_SMART_CARD 11
#define DEVICE_TYPE_SYSTEM_DEVICE 12
#define DEVICE_TYPE_SD_HOST_CONTROLLER 13
#define DEVICE_TYPE_INPUT_CONTROLLER 14
#define DEVICE_TYPE_KEYBOARD 15
#define DEVICE_TYPE_DOCKING_STATION 16
#define DEVICE_TYPE_PROCESSOR 17
#define DEVICE_TYPE_SERIAL_BUS 18
#define DEVICE_TYPE_DRIVE 19
#define DEVICE_TYPE_FS_PSEUDODEVICE 20

#define DEVICE_ID_CLASS(x) x.class
#define DEVICE_ID_CLASS_NONE 0
#define DEVICE_CREATE_ID_NONE (device_id_t){ .class = DEVICE_ID_CLASS_NONE }

#define DEVICE_ID_LIST_END DEVICE_CREATE_ID_NONE

//PS/2 Class

#define DEVICE_ID_CLASS_PS2 0x01

#define DEVICE_CREATE_ID_PS2 (device_id_t){ .class = DEVICE_ID_CLASS_PS2 }

//PCI Device Class

#define DEVICE_ID_CLASS_PCI 0x02

#define DEVICE_ID_PCI_VENDOR(x) x.value16[0]
#define DEVICE_ID_PCI_VENDOR_ANY 0xFFFF
#define DEVICE_ID_PCI_DEVICE(x) x.value16[1]
#define DEVICE_ID_PCI_DEVICE_ANY 0xFFFF
#define DEVICE_ID_PCI_CLASS(x) x.value16[2]
#define DEVICE_ID_PCI_SUBCLASS(x) x.value16[3]
#define DEVICE_ID_PCI_PROGIF(x) x.value8[0]
#define DEVICE_ID_PCI_PROGIF_ANY 0xFF

#define DEVICE_CREATE_ID_PCI(vid, did, cls, scls, progif) (device_id_t){ .class = DEVICE_ID_CLASS_PCI, .value16[0] = vid, .value16[1] = did, .value16[2] = cls, .value16[3] = scls, .value8[0] = progif }

//ACPI Class

#define DEVICE_ID_CLASS_ACPI 0x03

#define DEVICE_ID_ACPI_ID(x) x.string[0]

#define DEVICE_CREATE_ID_ACPI(id) (device_id_t){ .class = DEVICE_ID_CLASS_ACPI, .string[0] = id }

//Bus Class

#define DEVICE_ID_CLASS_BUS 0x04

#define DEVICE_ID_BUS_TYPE(x) x.value8[0]
#define DEVICE_ID_BUS_PCI 0x00

#define DEVICE_CREATE_ID_BUS(bus) (device_id_t){ .class = DEVICE_ID_CLASS_BUS, .value8[0] = bus }

//Timer Class

#define DEVICE_ID_CLASS_TIMER 0x05

#define DEVICE_ID_TIMER_TYPE(x) x.value8[0]
#define DEVICE_ID_TIMER_PIT 0x00

#define DEVICE_CREATE_ID_TIMER(timer) (device_id_t){ .class = DEVICE_ID_CLASS_TIMER, .value8[0] = timer }

//Filesystem Pseudodevice Class

#define DEVICE_ID_CLASS_FS_PDEV 0x06

#define DEVICE_CREATE_ID_FS_PDEV (device_id_t){ .class = DEVICE_ID_CLASS_FS_PDEV }
