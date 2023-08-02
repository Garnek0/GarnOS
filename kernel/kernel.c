#include <types.h>
#include <limine.h>

#include "video/fb.h"

// Halt function.
static void halt(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {
    fb_pixel(0,0,0xff00ffff);
    halt();
}
