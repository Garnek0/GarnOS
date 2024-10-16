#include "term-internals.h"

#include <garn/input.h>
#include <garn/mm.h>
#include <garn/kerrno.h>
#include <garn/kstdio.h>

kb_state_t keyboardState;

input_kb_rb_entry_t kbringbuffer[INPUT_RB_SIZE];
input_kb_rb_entry_t* writeRBPos;
input_kb_rb_entry_t* readRBPos;

vnode_t* kbd;

ssize_t input_rb_read(size_t size, void* buf){
    size_t i = 0;
    while(writeRBPos != readRBPos && i != size){
        ((char*)buf)[i] = readRBPos->chr;
        readRBPos->chr = 0;
        readRBPos = readRBPos->next;
        i++;
    }
    return i;
}

int input_rb_close(vfs_t* self, vnode_t* file){
    return 0;
}

char keyBuffer;

void input_init(){
    for(int i = 0; i < INPUT_RB_SIZE-1; i++){
        kbringbuffer[i].next = &kbringbuffer[i+1];
    }
    kbringbuffer[INPUT_RB_SIZE-1].next = &kbringbuffer[0];
    writeRBPos = readRBPos = &kbringbuffer[0];
}

void input_send_key(kb_input_t input){
    if(input.special){
        switch(input.key){
            case INPUT_LEFT_ALT:
                if(!input.pressed) keyboardState.lAlt = false;
                else keyboardState.lAlt = true;
                break;
            case INPUT_RIGHT_ALT:
                if(!input.pressed) keyboardState.rAlt = false;
                else keyboardState.rAlt = true;
                break;
            case INPUT_LEFT_CTRL:
                if(!input.pressed) keyboardState.lCtrl = false;
                else keyboardState.lCtrl = true;
                break;
            case INPUT_RIGHT_CTRL:
                if(!input.pressed) keyboardState.rCtrl = false;
                else keyboardState.rCtrl = true;
                break;
            case INPUT_LEFT_SHIFT:
                if(!input.pressed) keyboardState.lShift = false;
                else keyboardState.lShift = true;
                break;
            case INPUT_RIGHT_SHIFT:
                if(!input.pressed) keyboardState.rShift = false;
                else keyboardState.rShift = true;
                break;
            case INPUT_CAPSLOCK:
                if(keyboardState.capsLock && input.pressed) keyboardState.capsLock = false;
                else keyboardState.capsLock = true;
                break;
            default:
                break;
        }
    } else {
        //TODO: remove keyBuffer and kreadline after removing kcon
        if(input.pressed) keyBuffer = input.key;
        if(input.pressed){
            writeRBPos->chr = input.key;
            writeRBPos = writeRBPos->next;
        }
    }
}
