#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef signed long ssize_t;

uint32_t strlen(const char *s){
    uint32_t count = 0;
    while(*s!=0)
    {
        count++;
        s++;
    }
    return count;
}

ssize_t write(int fd, const void* buf, size_t count){
    asm volatile("movq $1, %%rax\n"
                 "movl %0, %%edi\n"
                 "movq %1, %%rsi\n"
                 "movq %2, %%rdx\n"
                 "int $0x80" : : "r"(fd), "r"(buf), "r"(count) : "%rax", "%rdi", "%rsi", "%rdx");
}

ssize_t read(int fd, const void* buf, size_t count){
    asm volatile("movq $0, %%rax\n"
                 "movl %0, %%edi\n"
                 "movq %1, %%rsi\n"
                 "movq %2, %%rdx\n"
                 "int $0x80" : : "r"(fd), "r"(buf), "r"(count) : "%rax", "%rdi", "%rsi", "%rdx");
}

int open(char* pathname, int flags, int mode){
    asm volatile("movq $2, %%rax\n"
                 "movq %0, %%rdi\n"
                 "movl %1, %%esi\n"
                 "movl %2, %%edx\n"
                 "int $0x80" : : "r"(pathname), "r"(flags), "r"(mode) : "%rax", "%rdi", "%esi", "%edx");
}

char* getcwd(const char* buf, size_t size){
    asm volatile("movq $79, %%rax\n"
                 "movq %0, %%rdi\n"
                 "movq %1, %%rsi\n"
                 "int $0x80" : : "r"(buf), "r"(size) : "%rax", "%rdi", "%rsi");
}

int chdir(const char* path){
    asm volatile("movq $80, %%rax\n"
                 "movq %0, %%rdi\n"
                 "int $0x80" : : "r"(path) : "%rax", "%rdi");
}

int fork(){
    asm volatile("movq $57, %%rax\n"
                 "int $0x80" : : : "%rax");
}

void exit(int status){
    asm volatile("movq $60, %%rax\n"
                 "movl %0, %%edi\n"
                 "int $0x80" : : "r"(status) : "%rax", "%edi");
}

void _start(){
    char* bufWelcome = "\ninit: init process is running! You can type stuff!\n";
    char* bufChildSuccess = "\nSuccessfully forked off child!\n";
    char* exitFail = "If you're seeing this then exit() failed :(\n";
    char key;

    write(1, bufWelcome, strlen(bufWelcome));

    if(fork() == 0){
        write(1, bufChildSuccess, strlen(bufChildSuccess));
        exit(0);
        write(1, exitFail, strlen(bufChildSuccess));
        while(1);
    }

    //flush stdin
    while(read(0, &key, 1));
    key = 0;

    
    for(;;){
        read(0, &key, 1);
        write(1, &key, 1);
        key = 0;
    }
}