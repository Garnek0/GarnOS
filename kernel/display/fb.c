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

framebuffer_info_t framebuffer_info;

//TODO: move to sys/bootloader.c
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

//put a single pixel on the screen
void fb_pixel(uint32_t x, uint32_t y, uint32_t colour){
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    uint32_t* fbPtr = framebuffer->address;
    fbPtr[y*(framebuffer->pitch/(framebuffer->bpp/8)) + x] = colour;
}

//clear the screen with a solid colour
void fb_clear(uint32_t colour){
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    uint32_t* fbPtr = framebuffer->address;
    for(int i = 0; i < framebuffer->height; i++){
        for(int j = 0; j < framebuffer->pitch/(framebuffer->bpp/8); j++){
            fbPtr[(framebuffer->pitch/(framebuffer->bpp/8)*i)+j] = colour;
        }
    }
}

//initialise the framebuffer
void fb_init(){
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    framebuffer_info.address = (uint64_t*)framebuffer->address;
    framebuffer_info.bpp = framebuffer->bpp;
    framebuffer_info.pitch = framebuffer->pitch;
    //calculate width using the pitch (it needs to be like this because on
    //some systems it causes issues with drawing due to the width provided
    //by Limine being incorrect [tested on a 2013 Fujitsu Laptop])
    framebuffer_info.width = framebuffer->pitch/(framebuffer->bpp/8);
    framebuffer_info.heigth = framebuffer->height;
}
