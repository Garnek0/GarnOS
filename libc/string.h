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

int strcmp(const char* s1, const char* s2){
   while (*s1 != '\0' && *s2 != '\0'  && *s1 == *s2) {
      s1++;
      s2++;
   }
   return *s1 - *s2;
}

int strncmp(const char* s1, const char* s2, size_t n){
   while (n != 0 && *s1 != '\0' && *s2 != '\0'  && *s1 == *s2) {
      s1++;
      s2++;
      n--;
   }
   return n == 0 ? 0 : (*s1 - *s2);
}

#endif //GARN_STRING_H