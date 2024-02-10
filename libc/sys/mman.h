#ifndef GARN_MMAN_H
#define GARN_MMAN_H

#define PROT_READ 1
#define PROT_WRITE (1 << 1)
#define PROT_EXEC (1 << 2)
#define PROT_NONE 0

#define MAP_SHARED 1
#define MAP_PRIVATE (1 << 1)
#define MAP_32BIT (1 << 6)
#define MAP_ANONYMOUS (1 << 5)
#define MAP_ANON MAP_ANONYMOUS

#include "../kernel/types.h"

void* mmap(void* addr, size_t length, int prot, int flags, int fd, uint64_t offset){
    void* ret = 0;

    asm volatile("movq $9, %%rax\n"
                 "movq %1, %%rdi\n"
                 "movq %2, %%rsi\n"
                 "movl %3, %%edx\n"
                 "movl %4, %%r10d\n"
                 "movl %5, %%r8d\n"
                 "movq %6, %%r9\n"
                 "int $0x80\n"
                 "movq %%rax, %0" : "=r"(ret) : "r"(addr), "r"(length), "r"(prot), "r"(flags), "r"(fd), "r"(offset) : "%rax", "%rdi", "%rsi", "%edx", "%r10", "%r8", "%r9");
}

int munmap(void* addr, size_t length){
    int ret = 0;

    asm volatile("movq $11, %%rax\n"
                 "movq %1, %%rdi\n"
                 "movq %2, %%rsi\n"
                 "int $0x80\n"
                 "movl %%eax, %0" : "=r"(ret) : "r"(addr), "r"(length) : "%rax", "%rdi", "%rsi");

    return ret;
}

#endif //GARN_MMAN_H