#ifndef GARN_UNISTD_H
#define GARN_UNISTD_H

#include "../kernel/types.h"

ssize_t read(int fd, const void* buf, size_t count){
    ssize_t ret = 0;

    asm volatile("movq $0, %%rax\n"
                 "movl %1, %%edi\n"
                 "movq %2, %%rsi\n"
                 "movq %3, %%rdx\n"
                 "int $0x80\n"
                 "movq %%rax, %0" : "=r"(ret) : "r"(fd), "r"(buf), "r"(count) : "%rax", "%rdi", "%rsi", "%rdx");

    return ret;
}

ssize_t write(int fd, const void* buf, size_t count){
    ssize_t ret = 0;

    asm volatile("movq $1, %%rax\n"
                 "movl %1, %%edi\n"
                 "movq %2, %%rsi\n"
                 "movq %3, %%rdx\n"
                 "int $0x80\n"
                 "movq %%rax, %0" : "=r"(ret) : "r"(fd), "r"(buf), "r"(count) : "%rax", "%rdi", "%rsi", "%rdx");

    return ret;
}

int open(char* pathname, int flags, int mode){
    int ret = 0;

    asm volatile("movq $2, %%rax\n"
                 "movq %1, %%rdi\n"
                 "movl %2, %%esi\n"
                 "movl %3, %%edx\n"
                 "int $0x80\n"
                 "movl %%eax, %0" : "=r"(ret) : "r"(pathname), "r"(flags), "r"(mode) : "%rax", "%rdi", "%esi", "%edx");

    return ret;
}

int close(int fd){
    int ret = 0;

    asm volatile("movq $3, %%rax\n"
                 "movl %0, %%edi\n"
                 "int $0x80\n"
                 "movl %%eax, %0" : "=r"(ret) : "r"(fd) : "%rax", "%edi");

    return ret;
}

int waitpid(int64_t pid, int* status, int options){
    int ret = 0;

    asm volatile("movq $7, %%rax\n"
                 "movq %1, %%rdi\n"
                 "movq %2, %%rsi\n"
                 "movl %3, %%edx\n"
                 "int $0x80\n"
                 "movl %%eax, %0" : "=r"(ret) : "r"(pid), "r"(status), "r"(options) : "%rax", "%rdi", "%rsi", "%edx");

    return ret;
}

int execve(const char* path, const char* argv[], const char* envp[]){
    int ret = 0;

    asm volatile("movq $59, %%rax\n"
                 "movq %1, %%rdi\n"
                 "movq %2, %%rsi\n"
                 "movq %3, %%rdx\n"
                 "int $0x80\n"
                 "movl %%eax, %0" : "=r"(ret) : "r"(path), "r"(argv), "r"(envp) : "%rax", "%rdi", "%rsi", "%rdx");

    return ret;
}

char* getcwd(const char* buf, size_t size){
    int ret = 0;

    asm volatile("movq $79, %%rax\n"
                 "movq %1, %%rdi\n"
                 "movq %2, %%rsi\n"
                 "int $0x80\n"
                 "movl %%eax, %0" : "=r"(ret) : "r"(buf), "r"(size) : "%rax", "%rdi", "%rsi");

    return ret;
}

int chdir(const char* path){
    int ret = 0;

    asm volatile("movq $80, %%rax\n"
                 "movq %1, %%rdi\n"
                 "int $0x80\n"
                 "movl %%eax, %0" : "=r"(ret) : "r"(path) : "%rax", "%rdi");

    return ret;
}

int fork(){
    int ret = 0;

    asm volatile("movq $57, %%rax\n"
                 "int $0x80\n"
                 "movl %%eax, %0" : "=r"(ret) : : "%rax");

    return ret;
}

void exit(int status){
    asm volatile("movq $60, %%rax\n"
                 "movl %0, %%edi\n"
                 "int $0x80" : : "r"(status) : "%rax", "%edi");
}

#endif //GARN_UNISTD_H