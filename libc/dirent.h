#ifndef GARN_DIRENT_H
#define GARN_DIRENT_H

#include "../kernel/types.h"

struct garn_dirent64 {
    uint64_t recordOffset;
    uint32_t recordLength;
    uint32_t type;
    char name[1];
} __attribute__((packed));

ssize_t getdents64(int fd, void* dirp, size_t count){
    ssize_t ret = 0;

    asm volatile("movq $217, %%rax\n"
                 "movl %1, %%edi\n"
                 "movq %2, %%rsi\n"
                 "movq %3, %%rdx\n"
                 "int $0x80\n"
                 "movq %%rax, %0" : "=r"(ret) : "r"(fd), "r"(dirp), "r"(count) : "%rax", "%edi", "%rsi", "%rdx");

    return ret;
}

#endif //GARN_DIRENT_H