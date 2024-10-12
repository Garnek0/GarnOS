#include <garn/kstdio.h>
#include <garn/term/term.h>
#include <garn/mm.h>
#include <garn/input.h>
#include <garn/kerrno.h>

spinlock_t kreadlineLock;

char* kreadline(char* prompt){
    if(prompt) kprintf(prompt);
    char* line = kmalloc(256);
    size_t i = 0;
    size_t backspaces = 0;

    char chr;

    keyBuffer = 0;

    lock(kreadlineLock, {
        kerrno = 0;
        while(keyBuffer!='\n'){
            keyBuffer = 0;
            while(!keyBuffer) arch_no_op();
            if(keyBuffer == '\b' && backspaces){
                kputchar(keyBuffer);
                backspaces--;
                i--;
                continue;
            } else if (keyBuffer == '\b' && !backspaces){
                continue;
            }
            chr = kputchar(keyBuffer);
            if(tc.escape || chr == 0) continue;
            line[i] = keyBuffer;
            i++;
            backspaces++;
            if(i > 255){
                kprintf("\n");
                klog("kreadline Buffer Overflow!\n", KLOG_WARNING, "klibc");
                kerrno = EOVERFLOW;
                line[0] = 0;
                break;
            }
        }
        line[i-1] = '\0';
    
    });
    return line;
}
