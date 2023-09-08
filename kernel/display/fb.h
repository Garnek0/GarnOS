/*  
*   File: fb.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FB_H
#define FB_H

#include <types.h>

typedef struct {
    uint32_t* address;
    uint32_t pitch;
    uint32_t width;
    uint32_t heigth;
    uint16_t bpp;
} framebuffer_info_t;
extern framebuffer_info_t framebuffer_info;

void fb_init();
void fb_pixel(uint32_t x, uint32_t y, uint32_t colour);
void fb_clear(uint32_t colour);

#endif //FB_H
