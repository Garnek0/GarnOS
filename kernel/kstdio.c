#include "kstdio.h"
#include <term/term.h>
#include <mem/memutil/memutil.h>
#include <mem/mm/kheap.h>

//unbuffered stdin. will most likely be changed
char stdin;

int kprintf(char* str, ...){
    va_list args;
    va_start(args, str);
    int chars = kvprintf(str, args);
    va_end(args);
    return chars;
}

static char workBuffer[256];
int kvprintf(char* str, va_list args){
    int chars = 0;

    for(int i = 0; i < strlen(str); i++){
        if(str[i] == '%'){
            switch(str[++i]){
                case '%':
                    //%
                    kputchar('%');
                    chars++;                    
                    break;
                case 'd':
                    //Signed decimal integer
                    int dnum = va_arg(args, int64_t);
                    int dnumCopy = dnum;
                    uint8_t dsize = 0;
                    uint8_t disNegative = 0;

                    if(dnum < 0){
                        dnum = -dnum;
                        dnumCopy = -dnumCopy;
                        disNegative = 1;
                        workBuffer[0] = '-';
                    }

                    do {
                        dnumCopy/=10;
                        dsize++;
                    } while(dnumCopy);

                    for(int i = 1; i <= dsize; i++){
                        workBuffer[disNegative + dsize - i] = dnum % 10 + '0';
                        dnum /= 10;
                    }
                    workBuffer[disNegative + dsize] = 0;
                    chars += kprintf(workBuffer);
                    break;
                case 'u':
                    //Unsigned decimal integer
                    uint64_t unum = va_arg(args, uint64_t);
                    uint64_t unumCopy = unum;
                    uint8_t usize = 0;

                    do {
                        unumCopy/=10;
                        usize++;
                    } while(unumCopy);

                    for(int i = 1; i <= usize; i++){
                        workBuffer[usize - i] = unum % 10 + '0';
                        unum /= 10;
                    }
                    workBuffer[usize] = 0;
                    chars += kprintf(workBuffer);
                    break;
                case 'c':
                    //Character
                    int cchr = va_arg(args, int);
                    kputchar(cchr);
                    chars++;
                    break;
                case 's':
                    //String
                    char* sstr = va_arg(args, char*);
                    chars += kprintf(sstr);
                    break;
                case 'x':
                    //Hex Number
                    uint64_t xnum = va_arg(args, uint64_t);
                    uint8_t* xvalPtr = &xnum;
                    uint8_t* xptr;
                    uint8_t xtmp;
                    uint8_t xsize = 1*2-1;
                    if(xnum > 0xFFFFFFFFFFFFFF){
                        xsize = 8*2-1;
                    } else if(xnum > 0xFFFFFFFFFFFF){
                        xsize = 7*2-1;
                    } else if(xnum > 0xFFFFFFFFFF){
                        xsize = 6*2-1;
                    } else if(xnum > 0xFFFFFFFF){
                        xsize = 5*2-1;
                    } else if(xnum > 0xFFFFFF){
                        xsize = 4*2-1;
                    } else if(xnum > 0xFFFF){
                        xsize = 3*2-1;
                    } else if(xnum > 0xFF){
                        xsize = 2*2-1;
                    }
                    for (uint8_t i = 0; i < xsize; i++){
                        xptr = ((uint8_t*)xvalPtr + i);
                        xtmp = ((*xptr & 0xF0) >> 4);
                        workBuffer[xsize - (i * 2 + 1)] = xtmp + (xtmp > 9 ? 55 : '0');
                        xtmp = ((*xptr & 0x0F));
                        workBuffer[xsize - (i * 2)] = xtmp + (xtmp > 9 ? 55 : '0');
                    }
                    workBuffer[xsize + 1] = 0;
                    chars += kprintf(workBuffer);
                    break;
                case 'p':
                    //Address
                    uint64_t paddress = va_arg(args, void*);
                    chars += kprintf("%x", paddress);
                    break;
            }
            continue;
        }
        kputchar(str[i]);
        chars++;
    }
    return chars;
}

char kputchar(char chr){
    return term_putchar(chr);
}

void klog(char* str, uint8_t loglevel, ...){

    va_list args;
    va_start(args, str);

    uint16_t r, g, b;
    uint32_t colour = term_get_colour(FOREGROUND);
    r = ((colour >> 16) & 0xff);
    g = ((colour >> 8) & 0xff);
    b = (colour & 0xff);

    switch(loglevel){
        case KLOG_OK:
            kprintf("[ \e[38;2;0;255;0mOK\e[38;2;%d;%d;%dm ] ", r, g, b, args);
            break;
        case KLOG_FAILED:
            kprintf("[ \e[38;2;255;0;0mFAILED\e[38;2;%d;%d;%dm ] ", r, g, b, args);
            break;
        case KLOG_INFO:
            kprintf("[ \e[38;2;0;255;255mINFO\e[38;2;%d;%d;%dm ] ", r, g, b, args);
            break;
        case KLOG_WARNING:
            kprintf("[ \e[38;2;255;255;0mWARNING\e[38;2;%d;%d;%dm ] ", r, g, b, args);
            break;
        case KLOG_FATAL:
            kprintf("\n[ \e[38;2;140;0;0mFATAL\e[38;2;%d;%d;%dm ] ", r, g, b, args);
            break;
        default:
            break;
    }

    kvprintf(str, args);
    va_end(args);
}

char* kreadline(char* prompt){
    if(prompt) kprintf(prompt);
    char* line = kmalloc(256);
    size_t i = 0;
    size_t backspaces = 0;

    stdin = 0;
    while(stdin!='\n'){
        stdin = 0;
        while(!stdin) asm volatile("nop");
        if(stdin == '\b' && backspaces){
            kputchar(stdin);
            backspaces--;
            i--;
            continue;
        } else if (stdin == '\b' && !backspaces){
            continue;
        }
        kputchar(stdin);
        line[i] = stdin;
        i++;
        backspaces++;
        if(i > 255){
            kprintf("\n");
            klog("kreadline Buffer Overflow!\n", KLOG_WARNING);
            line[0] = 0;
            break;
        }
    }
    line[i-1] = '\0';
    return line;
}