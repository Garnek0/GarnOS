#include <mem/mm-internals.h>

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
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

uint32_t strlen(const char *s){
    uint32_t count = 0;
    while(*s!=0)
    {
        count++;
        s++;
    }
    return count;
}

char* strdup(const char* str1){
    char* str2 = kmalloc(strlen(str1)+1);
    memcpy(str2, str1, strlen(str1)+1);
    return str2;
}

char *strncat(char *s1, const char *s2, size_t n){
    char *start = s1;

    while(*s1++);
    s1--;

    while(n--){
        if(!(*s1++ = *s2++)) return start;
    }
    *s1 = 0;

    return start;
}
