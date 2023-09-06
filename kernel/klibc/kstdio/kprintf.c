#include <kstdio.h>
#include <mem/memutil/memutil.h>

//unbuffered stdin. will most likely be changed
char stdin;

int kprintf(char* str, ...){
    va_list args;
    va_start(args, str);
    int chars = kvprintf(str, args);
    va_end(args);
    return chars;
}