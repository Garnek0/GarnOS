#include "fb.h"
#include <limine.h>

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

void fb_pixel(uint32_t x, uint32_t y, uint32_t colour){
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    uint32_t* fbPtr = framebuffer->address;
    fbPtr[y*(framebuffer->width) + x] = colour;
}
