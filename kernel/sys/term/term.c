/*  
*   File: term.c
*
*   Author: Garnek
*   
*   Description: Terminal Emulator
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "term.h"
#include "kernel.h"

#include <display/fb.h>
#include <mem/kheap/kheap.h>
#include <hw/ports.h>
#include <hw/serial/serial.h>

#include <mem/memutil/memutil.h>

term_context_t tc;

// Very incomplete teminal emulator

//colour lookup table. From flanterm (by mintsuki)
static const uint32_t col256[] = {
    0x000000, 0x00005f, 0x000087, 0x0000af, 0x0000d7, 0x0000ff, 0x005f00, 0x005f5f,
    0x005f87, 0x005faf, 0x005fd7, 0x005fff, 0x008700, 0x00875f, 0x008787, 0x0087af,
    0x0087d7, 0x0087ff, 0x00af00, 0x00af5f, 0x00af87, 0x00afaf, 0x00afd7, 0x00afff,
    0x00d700, 0x00d75f, 0x00d787, 0x00d7af, 0x00d7d7, 0x00d7ff, 0x00ff00, 0x00ff5f,
    0x00ff87, 0x00ffaf, 0x00ffd7, 0x00ffff, 0x5f0000, 0x5f005f, 0x5f0087, 0x5f00af,
    0x5f00d7, 0x5f00ff, 0x5f5f00, 0x5f5f5f, 0x5f5f87, 0x5f5faf, 0x5f5fd7, 0x5f5fff,
    0x5f8700, 0x5f875f, 0x5f8787, 0x5f87af, 0x5f87d7, 0x5f87ff, 0x5faf00, 0x5faf5f,
    0x5faf87, 0x5fafaf, 0x5fafd7, 0x5fafff, 0x5fd700, 0x5fd75f, 0x5fd787, 0x5fd7af,
    0x5fd7d7, 0x5fd7ff, 0x5fff00, 0x5fff5f, 0x5fff87, 0x5fffaf, 0x5fffd7, 0x5fffff,
    0x870000, 0x87005f, 0x870087, 0x8700af, 0x8700d7, 0x8700ff, 0x875f00, 0x875f5f,
    0x875f87, 0x875faf, 0x875fd7, 0x875fff, 0x878700, 0x87875f, 0x878787, 0x8787af,
    0x8787d7, 0x8787ff, 0x87af00, 0x87af5f, 0x87af87, 0x87afaf, 0x87afd7, 0x87afff,
    0x87d700, 0x87d75f, 0x87d787, 0x87d7af, 0x87d7d7, 0x87d7ff, 0x87ff00, 0x87ff5f,
    0x87ff87, 0x87ffaf, 0x87ffd7, 0x87ffff, 0xaf0000, 0xaf005f, 0xaf0087, 0xaf00af,
    0xaf00d7, 0xaf00ff, 0xaf5f00, 0xaf5f5f, 0xaf5f87, 0xaf5faf, 0xaf5fd7, 0xaf5fff,
    0xaf8700, 0xaf875f, 0xaf8787, 0xaf87af, 0xaf87d7, 0xaf87ff, 0xafaf00, 0xafaf5f,
    0xafaf87, 0xafafaf, 0xafafd7, 0xafafff, 0xafd700, 0xafd75f, 0xafd787, 0xafd7af,
    0xafd7d7, 0xafd7ff, 0xafff00, 0xafff5f, 0xafff87, 0xafffaf, 0xafffd7, 0xafffff,
    0xd70000, 0xd7005f, 0xd70087, 0xd700af, 0xd700d7, 0xd700ff, 0xd75f00, 0xd75f5f,
    0xd75f87, 0xd75faf, 0xd75fd7, 0xd75fff, 0xd78700, 0xd7875f, 0xd78787, 0xd787af,
    0xd787d7, 0xd787ff, 0xd7af00, 0xd7af5f, 0xd7af87, 0xd7afaf, 0xd7afd7, 0xd7afff,
    0xd7d700, 0xd7d75f, 0xd7d787, 0xd7d7af, 0xd7d7d7, 0xd7d7ff, 0xd7ff00, 0xd7ff5f,
    0xd7ff87, 0xd7ffaf, 0xd7ffd7, 0xd7ffff, 0xff0000, 0xff005f, 0xff0087, 0xff00af,
    0xff00d7, 0xff00ff, 0xff5f00, 0xff5f5f, 0xff5f87, 0xff5faf, 0xff5fd7, 0xff5fff,
    0xff8700, 0xff875f, 0xff8787, 0xff87af, 0xff87d7, 0xff87ff, 0xffaf00, 0xffaf5f,
    0xffaf87, 0xffafaf, 0xffafd7, 0xffafff, 0xffd700, 0xffd75f, 0xffd787, 0xffd7af,
    0xffd7d7, 0xffd7ff, 0xffff00, 0xffff5f, 0xffff87, 0xffffaf, 0xffffd7, 0xffffff,
    0x080808, 0x121212, 0x1c1c1c, 0x262626, 0x303030, 0x3a3a3a, 0x444444, 0x4e4e4e,
    0x585858, 0x626262, 0x6c6c6c, 0x767676, 0x808080, 0x8a8a8a, 0x949494, 0x9e9e9e,
    0xa8a8a8, 0xb2b2b2, 0xbcbcbc, 0xc6c6c6, 0xd0d0d0, 0xdadada, 0xe4e4e4, 0xeeeeee
};

void term_init(){
    tc.foregroundColour = 0xffffffff;
    tc.backgroundColour = 0x00000000;
    term_clear();
    tc.escape = tc.escapeCSI = false;
    tc.escapeOffset = 0;
    tc.enabled = true;

    kernel_screen_output_enable();
}

void term_clear(){
    fb_clear(tc.backgroundColour);
    cursor_set(&tc.cursor, 0, 0);
}

void term_disable(){
    tc.enabled = false;
}

void term_enable(){
    tc.enabled = true;
}

static void term_sgr(){
    switch(tc.escArgs[0]){
        case 38:
            if(tc.escArgs[1] == 5){
                tc.foregroundColour = col256[tc.escArgs[2]];
            } else if (tc.escArgs[1] == 2){
                tc.foregroundColour = (tc.escArgs[2] << 16) + (tc.escArgs[3] << 8) + tc.escArgs[4];
            }
            break;
        case 48:
            if(tc.escArgs[1] == 5){
                tc.backgroundColour = col256[tc.escArgs[2]];
            } else if (tc.escArgs[1] == 2){
                tc.backgroundColour = (tc.escArgs[2] << 16) + (tc.escArgs[3] << 8) + tc.escArgs[4];
            }
            break;
        default:
            break;
    }
}

void term_scroll(uint16_t pix){
        uint64_t cpyDest = (uint64_t)framebuffer_info.readAddress;
        uint64_t cpySrc = (framebuffer_info.pitch*pix)+(uint64_t)framebuffer_info.readAddress;
        size_t cpySize = framebuffer_info.size - framebuffer_info.pitch*pix;

        //scroll the read buffer
        memcpy((void*)cpyDest, (void*)cpySrc, cpySize);

        //zero out the last row of (framebuffer_info.pitch*pix) pixels
        memset((void*)(framebuffer_info.size-framebuffer_info.pitch*pix+(uint64_t)framebuffer_info.readAddress), 0xffffffff, framebuffer_info.pitch*pix);
        for(int i = 0; i < framebuffer_info.width*pix; i++){
            ((uint32_t*)(framebuffer_info.size-framebuffer_info.pitch*pix+(uint64_t)framebuffer_info.readAddress))[i] = tc.backgroundColour;
        }

        //copy the read buffer to the actual framebuffer
        memcpy((void*)framebuffer_info.address, (void*)framebuffer_info.readAddress, framebuffer_info.size);
}

static void term_putchar_raw(char chr){

    if(!tc.enabled) return;

    outb(0xE9, (uint8_t)chr);
    serial_write((uint8_t)chr);
    if(chr == '\n') serial_write((uint8_t)'\r');

    switch(chr){
        case '\n':
            cursor_newline(&tc.cursor);
            // fall through
        case '\r':
            cursor_return(&tc.cursor);
            break;
        default:
            for(uint32_t i = tc.cursor.posY; i < tc.cursor.posY+GLYPH_Y; i++){
                for(uint32_t j = tc.cursor.posX; j < tc.cursor.posX+GLYPH_X; j++){
                    fb_pixel(j, i, tc.backgroundColour);
                    if(font[chr*GLYPH_Y+i-tc.cursor.posY] & (0b10000000 >> (j-tc.cursor.posX))){
                        fb_pixel(j, i, tc.foregroundColour);
                    }
                }
            }
            cursor_advance(&tc.cursor);
            break;
    }
}

static void term_handle_esc(char chr){
    if(tc.escapeOffset == 1){
        if(chr == '[') tc.escapeCSI = true;
        return;
    }

    if(chr >= '0' && chr <= '9'){
        if(tc.escArgsCount == 16) return;
        tc.escArgs[tc.escArgsCount] *= 10;
        tc.escArgs[tc.escArgsCount] += chr - '0';
        return;
    } else if (chr == ';'){
        tc.escArgsCount++;
        return;
    }

    switch(chr){
        case '@':
            for(uint8_t i = 0; i < tc.escArgs[0]; i++){
                term_putchar_raw(' ');
            }
            break;
        case 'm':
            term_sgr();
            break;
        case 'J':
            if(tc.escArgsCount == 0){
                if(tc.escArgs[0] == 2){
                    term_clear();
                }
            }
        default:
            break;
    }

    tc.escape = tc.escapeCSI = false;
    tc.escapeOffset = 0;
    for(int i = 0; i < 16; i++){
        tc.escArgs[i] = 0;
    }
    tc.escArgsCount = 0;
    return;
}

char term_putchar(char chr){
    if(chr == 0) return 0;

    if(!tc.enabled) return;

    lock(tc.lock, {
        outb(0xE9, (uint8_t)chr);
        serial_write((uint8_t)chr);
        if(chr == '\n') serial_write((uint8_t)'\r');

        if(tc.escape){
            tc.escapeOffset++;
            term_handle_esc(chr);
            releaseLock(&tc.lock);
            return 0;
        }

        switch(chr){
            case '\n':
                cursor_newline(&tc.cursor);
                // fall through
            case '\r':
                cursor_return(&tc.cursor);
                break;
            case '\e':
                tc.escape = true;
                break;
            case '\b':
                releaseLock(&tc.lock);
                cursor_backspace(&tc.cursor);
                break;
            default:
                for(int i = tc.cursor.posY; i < tc.cursor.posY+GLYPH_Y; i++){
                    for(int j = tc.cursor.posX; j < tc.cursor.posX+GLYPH_X; j++){
                        fb_pixel(j, i, tc.backgroundColour);
                        if(font[chr*GLYPH_Y+i-tc.cursor.posY] & (0b10000000 >> j-tc.cursor.posX)){
                            fb_pixel(j, i, tc.foregroundColour);
                        }
                    }
                }
                cursor_advance(&tc.cursor);
                break;
        }
    });
    return chr;
}

char term_putchar_dbg(char chr){
    lock(tc.lock, {
        outb(0xE9, (uint8_t)chr);
        serial_write((uint8_t)chr);
        if(chr == '\n') serial_write((uint8_t)'\r');
    });
    return chr;
}

int term_print(char* str){
    int chars = 0;
    for(int i = 0; i < (int)strlen(str); i++){
        term_putchar(str[i]);
        chars++;
    }
    return chars;
}

int term_print_dbg(char* str){
    int chars = 0;
    for(int i = 0; i < (int)strlen(str); i++){
        term_putchar_dbg(str[i]);
        chars++;
    }
    return chars;
}

uint32_t term_get_colour(bool foreback){
    if(!foreback) return tc.foregroundColour;
    return tc.backgroundColour;
}

void term_set_colour(uint32_t colour, bool foreback){
    if(!foreback){
        tc.foregroundColour = colour;
        return;
    }
    tc.backgroundColour = colour;
}   
