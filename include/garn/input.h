#pragma once

#define INPUT_LEFT_SHIFT 0x00
#define INPUT_RIGHT_SHIFT 0x01
#define INPUT_LEFT_ALT 0x02
#define INPUT_RIGHT_ALT 0x03
#define INPUT_LEFT_CTRL 0x04
#define INPUT_RIGHT_CTRL 0x05
#define INPUT_CAPSLOCK 0x06

#define INPUT_RB_SIZE 32

#include <garn/types.h>
#include <garn/fal/vnode.h>
#include <garn/fal/vfs.h>

typedef struct _input_kb_rb_entry {
    char chr;
    struct _input_kb_rb_entry* next;
} input_kb_rb_entry_t;

typedef struct {
    bool lShift;
    bool rShift;
    bool lCtrl;
    bool rCtrl;
    bool lAlt;
    bool rAlt;
    bool capsLock;
} kb_state_t;
extern kb_state_t keyboardState;

typedef struct {
    char key;
    bool pressed; //true - key is pressed, false - key is released
    bool special; //if this is true, then "key" is treated as a special key 
} kb_input_t;

extern char keyBuffer;

extern vnode_t* kbd;

void input_send_key(kb_input_t input);
