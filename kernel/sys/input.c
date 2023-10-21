/*  
*   File: input.c
*
*   Author: Garnek
*   
*   Description: Input Handling System
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "input.h"

kb_state_t keyboardState;

char keyBuffer;

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
        if(input.pressed) keyBuffer = input.key;
    }
}