#include <kstdio.h>

static char workBuffer[256];
int kvprintf(char* fmt, va_list args){
    int chars = 0;

    for(int i = 0; i < strlen(fmt); i++){
        if(fmt[i] == '%'){
            switch(fmt[++i]){
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
        kputchar(fmt[i]);
        chars++;
    }
    return chars;
}