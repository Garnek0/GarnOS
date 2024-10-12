#include <garn/kstdio.h>
#include <garn/mm.h>

spinlock_t kprintfLock;

int kprintf(char* str, ...){
    va_list args;
    va_start(args, str);
    int chars;

    lock(kprintfLock, {
        chars = kvprintf(str, args);
        va_end(args);
    });
    return chars;
}
