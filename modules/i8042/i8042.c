/*  
*   Module: i8042.sys
*
*   File: i8042.c
*
*   Module Author: Garnek
*   
*   Module Description: i8042 PS/2 Controller Driver
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "i8042.h"

void i8042_write(uint8_t port, uint8_t data){
    bool timedOut = true;
    for(int i = 0; i < 5; i++){
        if(!(arch_inb(i8042_COMMAND) & i8042_STATUS_WRITERDY)){
            timedOut = false;
            break;
        }
        ksleep(1);
    }
    if(timedOut){
        klog("i8042 Write Access Timed Out.\n", KLOG_WARNING, "i8042");
        return;
    }
    arch_outb(port, data);
}

uint8_t i8042_read(uint8_t port){
    bool timedOut = true;
    for(int i = 0; i < 5; i++){
        if(arch_inb(i8042_COMMAND) & i8042_STATUS_READRDY){
            timedOut = false;
            break;
        }
        ksleep(10);
    }
    if(timedOut){
        klog("i8042 Read Access Timed Out.\n", KLOG_WARNING, "i8042");
        return 0;
    }
    return arch_inb(port);
}

void init(){
    return;
}

void fini(){
	i8042_kb_fini();
    return;
}

bool probe(device_t* device){
    if(device->type == DEVICE_TYPE_INPUT_CONTROLLER && device->bus == DEVICE_BUS_NONE) return true;
    return false;
}

bool attach(device_t* device){
    if(!probe(device)) return false;

    uint8_t configByte; //used to store the configuration byte
    uint8_t res; //used to store test results
    bool dualChannel = false; //self explanatory

    arch_inb(i8042_DATA); //Discard leftover data

    i8042_write(i8042_COMMAND, i8042_COMMAND_DISABLE_PORT1);
    i8042_write(i8042_COMMAND, i8042_COMMAND_DISABLE_PORT2);

    arch_inb(i8042_DATA); //Discard leftover data

    i8042_write(i8042_COMMAND, i8042_COMMAND_GET_CONFIG);
    configByte = i8042_read(i8042_DATA);
    if(configByte & i8042_CONFIG_PORT2_CLK) dualChannel = true;
    configByte &= ~(i8042_CONFIG_PORT1_IEN | i8042_CONFIG_PORT2_IEN | i8042_CONFIG_TRANSLATION);
    i8042_write(i8042_COMMAND, i8042_COMMAND_SET_CONFIG);
    i8042_write(i8042_DATA, configByte);

    i8042_write(i8042_COMMAND, i8042_COMMAND_SELFTEST);
    res = i8042_read(i8042_DATA);
    if(res != 0x55){
        klog("i8042 Controller Self-Test Failed! Test returned 0x%x. Assuming i8042 is nonexistent or faulty.\n", KLOG_WARNING, "i8042", res);
        return false;
    }

    i8042_write(i8042_COMMAND, i8042_COMMAND_TEST_PORT1);
    res = i8042_read(i8042_DATA);
    if(res){
        klog("i8042 Port 1 Faulty! Test returned 0x%x.\n", KLOG_WARNING, "i8042", res);
    }

    if(dualChannel){
        i8042_write(i8042_COMMAND, i8042_COMMAND_TEST_PORT2);
        res = i8042_read(i8042_DATA);
        if(res){
            klog("i8042 Port 2 Faulty! Test returned 0x%x.\n", KLOG_WARNING, "i8042", res);
        }
    }

    i8042_write(i8042_COMMAND, i8042_COMMAND_ENABLE_PORT1);
    if(dualChannel) i8042_write(i8042_COMMAND, i8042_COMMAND_ENABLE_PORT2);

    configByte |= (i8042_CONFIG_PORT1_IEN | i8042_CONFIG_TRANSLATION);
    if(dualChannel) configByte |= i8042_CONFIG_PORT2_IEN;
    i8042_write(i8042_COMMAND, i8042_COMMAND_SET_CONFIG);
    i8042_write(i8042_DATA, configByte);

    klog("i8042 Controller Initialised.\n", KLOG_OK, "i8042");

    //controller initialised, now initialise devices

    i8042_kb_init();

    return true;
}

bool remove(device_t* device){
    i8042_kb_fini();
    return true;
}

module_t metadata = {
    .name = "i8042",
    .init = init,
    .fini = fini
};

device_driver_t driver_metadata = {
    .probe = probe,
    .attach = attach,
    .remove = remove
};

device_id_t driver_ids[] = {
    DEVICE_CREATE_ID_PS2,
    0
};
