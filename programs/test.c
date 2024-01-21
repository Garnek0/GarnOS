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

void _start(){
    char* bufWelcome = "\ntest: New process running! You can type stuff!\n";

    write(1, bufWelcome, strlen(bufWelcome));

    char key;

    //flush stdin
    while(read(0, &key, 1));
    key = 0;

    for(;;){
        read(0, &key, 1);
        write(1, &key, 1);
        key = 0;
    }
}