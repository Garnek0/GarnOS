/*  
*   Module: i8042.sys
*
*   File: i8042kb.c
*
*   Module Author: Garnek
*   
*   Module Description: i8042 PS/2 Keyboard driver
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "i8042.h"

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
    uint8_t status = i8042_read(i8042_COMMAND);
    if(!(status & i8042_STATUS_READRDY)) return; //Interrupt likely didn't come from the i8042 controller
    uint8_t scancode = i8042_read(i8042_DATA);
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

void i8042_kb_init(){
    //set scancode mode to 2
    i8042_write(i8042_DATA, 0xF0);
    i8042_read(i8042_DATA);
    i8042_write(i8042_DATA, 2);
    i8042_read(i8042_DATA);

    irq_add_handler(1, keyboard_handler, IRQ_SHARED);
    arch_inb(i8042_DATA); //Discard data one more time just in case
    klog("Keyboard Initialised\n", KLOG_OK, "i8042");
}

void i8042_kb_fini(){
    irq_remove_handler(1, keyboard_handler);
	klog("Keyboard Deinitialised\n", KLOG_OK, "i8042");
}
