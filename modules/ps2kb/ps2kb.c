#include <module/module.h>
#include <drivers/ports.h>
#include <cpu/interrupts/interrupts.h>
#include <kstdio.h>

#define PS2_DATA 0x60
#define PS2_COMMAND 0x64

#define LEFT_SHIFT 0x2A
#define LEFT_CTRL 0x1D
#define RIGHT_SHIFT 0x36
#define RIGHT_CTRL 0x1D
#define ENTER 0x1C
#define BACKSPACE 0x0E
#define CAPSLOCK 0x3A
#define EX_SCANCODE 0x0E

typedef struct {
    bool lShift;
    bool rShift;
    bool lCtrl;
    bool rCtrl;
    bool capsLock;
} kb_state_t;
kb_state_t state;

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
    uint8_t scancode = inb(PS2_DATA);
    switch(scancode){
        case LEFT_SHIFT:
            state.lShift = true;
            break;
        case LEFT_SHIFT + 0x80:
            state.lShift = false;
            break;
        case RIGHT_SHIFT:
            state.rShift = true;
            break;
        case RIGHT_SHIFT + 0x80:
            state.rShift = false;
            break;
        case LEFT_CTRL:
            state.lCtrl = true;
            break;
        case LEFT_CTRL + 0x80:
            state.lCtrl = false;
            break;
        case CAPSLOCK:
            if(state.capsLock){
                state.capsLock = false;
                break;
            }
            state.capsLock = true;
            break;
        case EX_SCANCODE:
            scancode = inb(0x60);
            switch(scancode){
                case RIGHT_CTRL:
                    state.rCtrl = true;
                    break;
                case RIGHT_CTRL + 0x80:
                    state.rCtrl = false;
                    break;
                default:
                    break;
            }
        default:
            if(scancode > 58) break;
            if(state.lShift || state.rShift || state.capsLock){
                stdin = keymapUppercase[scancode];
                break;
            }
            stdin = keymap[scancode];
            break;
    }
}


void init(){
    irqHandler.keyboard_handler = keyboard_handler;
}

void fini(){
    return;
}

module_t metadata = {
    .name = "8042ps2kb",
    .init = init,
    .fini = fini
};