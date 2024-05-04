/*  
*   Module: ps2.sys
*
*   File: ps2.c
*
*   Module Author: Garnek
*   
*   Module Description: PS/2 Controller Driver
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "ps2.h"
#include <module/module.h>
#include <cpu/interrupts/interrupts.h>
#include <mem/kheap/kheap.h>
#include <sys/input.h>

device_driver_t driver_metadata;

char keymap[] = {
     0,  '\e', '1', '2',
    '3', '4', '5', '6',
    '7', '8', '9', '0',
    '-', '=',  '\b',  0,
    'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i',
    'o', 'p', '[', ']',
     '\n',  0, 'a', 's',
    'd', 'f', 'g', 'h',
    'j', 'k', 'l', ';',
    '\'','`',  0, '\\',
    'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',',
    '.', '/',  0, '*',
     0, ' '
};

char keymapUppercase[] = {
     0,  '\e', '!', '@',
    '#', '$', '%', '^',
    '&', '*', '(', ')',
    '_', '+',  '\b',  0,
    'Q', 'W', 'E', 'R',
    'T', 'Y', 'U', 'I',
    'O', 'P', '{', '}',
     '\n',  0, 'A', 'S',
    'D', 'F', 'G', 'H',
    'J', 'K', 'L', ':',
    '\"','~',  0, '|',
    'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<',
    '>', '?',  0, '*',
     0, ' '
};

void keyboard_handler(stack_frame_t* frame){
    uint8_t scancode = ps2_read(PS2_DATA);
    kb_input_t input;
    switch(scancode){
        case LEFT_SHIFT:
            input.special = true;
            input.pressed = true;
            input.key = INPUT_LEFT_SHIFT;
            input_send_key(input);
            break;
        case LEFT_SHIFT + 0x80:
            input.special = true;
            input.pressed = false;
            input.key = INPUT_LEFT_SHIFT;
            input_send_key(input);
            break;
        case RIGHT_SHIFT:
            input.special = true;
            input.pressed = true;
            input.key = INPUT_RIGHT_SHIFT;
            input_send_key(input);
            break;
        case RIGHT_SHIFT + 0x80:
            input.special = true;
            input.pressed = false;
            input.key = INPUT_RIGHT_SHIFT;
            input_send_key(input);
            break;
        case LEFT_CTRL:
            input.special = true;
            input.pressed = true;
            input.key = INPUT_LEFT_CTRL;
            input_send_key(input);
            break;
        case LEFT_CTRL + 0x80:
            input.special = true;
            input.pressed = false;
            input.key = INPUT_LEFT_CTRL;
            input_send_key(input);
            break;
        case CAPSLOCK:
            input.special = true;
            input.pressed = true;
            input.key = INPUT_CAPSLOCK;
            input_send_key(input);
            break;
        case CAPSLOCK + 0x80:
            input.special = true;
            input.pressed = false;
            input.key = INPUT_CAPSLOCK;
            input_send_key(input);
            break;
        default:
            if(scancode <= 58){
                input.special = false;
                input.pressed = true;
            } else if ((scancode - 0x80) <= 58){
                scancode -= 0x80;
                input.special = false;
                input.pressed = false;
            } else {
                break;
            }

            if(keyboardState.lShift || keyboardState.rShift || keyboardState.capsLock) input.key = keymapUppercase[scancode];
            else input.key = keymap[scancode];
            input_send_key(input);
            break;
    }
}

void init(){
    
}

void fini(){
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

    asm volatile("cli");

    inb(PS2_DATA); //Discard leftover data

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

    irq_add_handler(1, keyboard_handler);
    klog("PS2: Keyboard Initialised\n", KLOG_OK);

    asm volatile("sti");

    klog("PS2: PS2 Controller Initialised.\n", KLOG_OK);

    return true;
}

bool remove(device_t* device){
    irq_add_handler(1, NULL);
    return true;
}

module_t metadata = {
    .name = "i8042ps2",
    .init = init,
    .fini = fini
};

device_driver_t driver_metadata = {
    .probe = probe,
    .attach = attach,
    .remove = remove
};

device_id_t* driver_ids[] = {
    DEVICE_CREATE_ID_PS2,
    0
};