#include <arch/x86_64/msr/msr.h>

uint64_t rdmsr(uint64_t msr){
    uint64_t rax, rdx;
    asm volatile("rdmsr" : "=a" (rax), "=d" (rdx) : "c"(msr));

    return (rdx << 32) | rax;
}

void wrmsr(uint64_t msr, uint64_t data){
    uint64_t rax = data & 0xFFFFFFFF;
    uint64_t rdx = data >> 32;
    asm volatile("wrmsr" :: "a" (rax), "d" (rdx), "c"(msr));
}
