#include <kstdio.h>
#include <term/term.h>

char kputchar(char chr){
    return term_putchar(chr);
}