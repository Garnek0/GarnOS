/*  
*   File: fb.c
*
*   Author: Garnek
*   
*   Description: Framebuffer
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fb.h"
#include <limine.h>
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>
#include <sys/term/term.h>
#include <cpu/multiproc/spinlock.h>
#include <sys/term/term.h>

framebuffer_info_t framebuffer_info;

spinlock_t fbLock;

//(Having this in bootloader.c seems to limine crash randomly...)
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

//put a single pixel on the screen
void fb_pixel(uint32_t x, uint32_t y, uint32_t colour){
    lock(fbLock, {
        uint32_t* fbPtr = framebuffer_info.address;
        uint32_t* fbReadPtr = framebuffer_info.readAddress;
        fbPtr[y*(framebuffer_info.pitch/(framebuffer_info.bpp/8)) + x] = colour;
        fbReadPtr[y*(framebuffer_info.pitch/(framebuffer_info.bpp/8)) + x] = colour;
    });
}

//clear the screen with a solid colour
void fb_clear(uint32_t colour){
    lock(fbLock, {
        uint32_t* fbPtr = framebuffer_info.address;
        uint32_t* fbReadPtr = framebuffer_info.readAddress;
        for(uint32_t i = 0; i < framebuffer_info.height; i++){
            for(uint32_t j = 0; j < framebuffer_info.pitch/(framebuffer_info.bpp/8); j++){
                fbPtr[(framebuffer_info.pitch/(framebuffer_info.bpp/8)*i)+j] = colour;
                fbReadPtr[(framebuffer_info.pitch/(framebuffer_info.bpp/8)*i)+j] = colour;
            }
        }
    });
}

//initialise the framebuffer
void fb_init(){
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    framebuffer_info.address = framebuffer_info.readAddress = (uint32_t*)framebuffer->address;
    framebuffer_info.bpp = framebuffer->bpp;
    framebuffer_info.pitch = framebuffer->pitch;
    //calculate width using the pitch (it needs to be like this because on
    //some systems it causes issues with drawing due to the width provided
    //by Limine being incorrect [tested on a 2013 Fujitsu Laptop])
    framebuffer_info.width = framebuffer->pitch/(framebuffer->bpp/8);
    framebuffer_info.height = framebuffer->height;
    framebuffer_info.size = framebuffer->height*framebuffer->pitch;
}

//reading from the framebuffer is slow. Make a copy and
//use that instead
void fb_read_init(){
    framebuffer_info.readAddress = kmalloc(framebuffer_info.height*framebuffer_info.pitch);
    memset(framebuffer_info.readAddress, 0, framebuffer_info.height*framebuffer_info.pitch);
    term_clear();
}
