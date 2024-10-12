/** @file fb.h
 * @brief Framebuffer Access
 * 
 * @author Garnek
 * @date 2024
 */


/*  
*   File: fb.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FB_H
#define FB_H

#include <garn/types.h>

/** @struct framebuffer_info_t
 * @brief Framebuffer information structure
 * 
 * @var framebuffer_info_t::address
 * Framebuffer base address
 * 
 * @var framebuffer_info_t::readAddress
 * Read Framebuffer base address
 * 
 * @var framebuffer_info_t::pitch
 * Framebuffer pitch (bytes per row)
 * 
 * @var framebuffer_info_t::width
 * Framebuffer width (in pixels)
 * 
 * @var framebuffer_info_t::heigth
 * Framebuffer height (in pixels)
 * 
 * @var framebuffer_info_t::bpp
 * Bytes per pixel
 * 
 * @var framebuffer_info_t::size
 * Framebuffer size (in bytes)
 */
typedef struct {
    uint32_t* address;
    uint32_t* readAddress;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint16_t bpp;
    size_t size;
} framebuffer_info_t;

/**
 * @brief Default framebuffer info structure
 * 
 */
extern framebuffer_info_t framebuffer_info;

/**
 * @brief Writes a sigle pixel to the framebuffer
 * 
 * @param x X offset
 * @param y Y offset
 * @param colour Pixel colour
 */
void fb_pixel(uint32_t x, uint32_t y, uint32_t colour);

/**
 * @brief Clears the framebuffer
 * 
 * @param colour Clear colour
 */
void fb_clear(uint32_t colour);

#endif //FB_H
