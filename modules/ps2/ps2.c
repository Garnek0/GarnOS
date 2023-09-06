#include "ps2.h"
#include <module/module.h>
#include <cpu/interrupts/interrupts.h>

void init(){

    uint8_t configByte; //used to store the configuration byte
    uint8_t res; //used to store test results
    bool dualChannel = false; //self explanatory

    asm volatile("cli");

    ps2_write(PS2_COMMAND, PS2_COMMAND_DISABLE_PORT1);
    ps2_write(PS2_COMMAND, PS2_COMMAND_DISABLE_PORT2);

    inb(PS2_DATA); //Discard leftover data

    ps2_write(PS2_COMMAND, PS2_COMMAND_GET_CONFIG);
    configByte = ps2_read(PS2_DATA);
    if(configByte & PS2_CONFIG_PORT2_CLK) dualChannel = true;
    configByte &= ~(PS2_CONFIG_PORT1_IEN | PS2_CONFIG_PORT2_IEN | PS2_CONFIG_TRANSLATION);
    ps2_write(PS2_COMMAND, PS2_COMMAND_SET_CONFIG);
    ps2_write(PS2_DATA, configByte);

    ps2_write(PS2_COMMAND, PS2_COMMAND_SELFTEST);
    res = ps2_read(PS2_DATA);
    if(res != 0x55){
        klog("PS2: PS2 Controller Self-Test Failed! Test returned 0x%x.\n", KLOG_WARNING, res);
        rb_log("PS2Controller", KLOG_FAILED);
        return;
    }

    ps2_write(PS2_COMMAND, PS2_COMMAND_TEST_PORT1);
    res = ps2_read(PS2_DATA);
    if(res){
        klog("PS2: PS2 Port 1 Faulty! Test returned 0x%x.\n", KLOG_WARNING, res);
    }

    if(dualChannel){
        ps2_write(PS2_COMMAND, PS2_COMMAND_TEST_PORT2);
        res = ps2_read(PS2_DATA);
        if(res){
            klog("PS2: PS2 Port 2 Faulty! Test returned 0x%x.\n", KLOG_WARNING, res);
        }
    }

    ps2_write(PS2_COMMAND, PS2_COMMAND_ENABLE_PORT1);
    if(dualChannel) ps2_write(PS2_COMMAND, PS2_COMMAND_ENABLE_PORT2);

    configByte |= (PS2_CONFIG_PORT1_IEN | PS2_CONFIG_TRANSLATION);
    if(dualChannel) configByte |= PS2_CONFIG_PORT2_IEN;
    ps2_write(PS2_COMMAND, PS2_COMMAND_SET_CONFIG);
    ps2_write(PS2_DATA, configByte);

    //set scancode mode to 2
    ps2_write(PS2_DATA, 0xF0);
    ps2_read(PS2_DATA);
    ps2_write(PS2_DATA, 2);
    ps2_read(PS2_DATA);

    asm volatile("sti");

    klog("PS2: PS2 Controller Initialised.\n", KLOG_OK);
    rb_log("PS2Controller", KLOG_OK);
}

void fini(){
    return;
}

module_t metadata = {
    .name = "8042ps2",
    .init = init,
    .fini = fini
};