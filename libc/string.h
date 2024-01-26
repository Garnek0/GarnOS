#ifndef GARN_STRING_H
#define GARN_STRING_H

#include "../kernel/types.h"

uint32_t strlen(const char *s){
    uint32_t count = 0;
    while(*s!=0)
    {
        count++;
        s++;
    }
    return count;
}

#endif //GARN_STRING_H