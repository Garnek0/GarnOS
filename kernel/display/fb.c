#include "fb.h"
#include <limine.h>

framebuffer_info_t framebuffer_info;

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

void fb_pixel(uint32_t x, uint32_t y, uint32_t colour){
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    uint32_t* fbPtr = framebuffer->address;
    fbPtr[y*(framebuffer->pitch/(framebuffer->bpp/8)) + x] = colour;
}

void fb_clear(uint32_t colour){
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    uint32_t* fbPtr = framebuffer->address;
    for(int i = 0; i < framebuffer->height; i++){
        for(int j = 0; j < framebuffer->pitch/(framebuffer->bpp/8); j++){
            fbPtr[(framebuffer->pitch/(framebuffer->bpp/8)*i)+j] = colour;
        }
    }
}

void fb_init(){
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    framebuffer_info.address = (uint64_t*)framebuffer->address;
    framebuffer_info.bpp = framebuffer->bpp;
    framebuffer_info.pitch = framebuffer->pitch;
    framebuffer_info.width = framebuffer->pitch/(framebuffer->bpp/8);
    framebuffer_info.heigth = framebuffer->height;
}
