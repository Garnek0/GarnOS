#ifndef MEMUTIL_H
#define MEMUTIL_H

#include <types.h>

#define ALIGN_UP(x, alignment) (((uint64_t)x % alignment == 0) ? (typeof(x))(x) : (typeof(x))((uint64_t)x + (alignment - ((uint64_t)x % alignment))))

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
uint32_t strlen(const char *s);

#endif //MEMUTIL_H