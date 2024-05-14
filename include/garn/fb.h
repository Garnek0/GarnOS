/*  
*   File: fb.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FB_H
#define FB_H

#include <garn/types.h>

typedef struct {
    uint32_t* address;
    uint32_t* readAddress;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint16_t bpp;
    size_t size;
} framebuffer_info_t;
extern framebuffer_info_t framebuffer_info;

void fb_pixel(uint32_t x, uint32_t y, uint32_t colour);
void fb_clear(uint32_t colour);

inline framebuffer_info_t fb_get_info(){
    return framebuffer_info;
}

inline void fb_set_info(framebuffer_info_t fbinfo){
    framebuffer_info = fbinfo;
}

#endif //FB_H
