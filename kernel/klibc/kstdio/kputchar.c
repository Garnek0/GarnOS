#include <garn/kstdio.h>
#include <garn/term/term.h>

//this is just a wrapper, it doesnt need a lock as
//the wrapped function (term_putchar) already has one

bool kernelScreenOut = false;

char kputchar(char chr){
    if(kernelScreenOut){
        return term_putchar(chr);
    } else {
        return term_putchar_dbg(chr);
    }
}

void kernel_screen_output_disable(){
    kernelScreenOut = false;
}

void kernel_screen_output_enable(){
    kernelScreenOut = true;
}
