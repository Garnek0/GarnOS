#include <garn/arch/common.h>
#include <garn/mm.h>
#include <cpuid.h>

void arch_get_cpu_model_name(char* str){
    uint32_t regs[13];

    __get_cpuid(0x80000000, &regs[0], &regs[1], &regs[2], &regs[3]);

    if (regs[0] < 0x80000004) return;

    __get_cpuid(0x80000002, &regs[0], &regs[1], &regs[2], &regs[3]);
    __get_cpuid(0x80000003, &regs[4], &regs[5], &regs[6], &regs[7]);
    __get_cpuid(0x80000004, &regs[8], &regs[9], &regs[10], &regs[11]);

    regs[12] = 0;

    char name[49];
    memcpy((void*)name, (void*)regs, 48);
    name[48] = 0;

    //Remove the spaces
    for(int i = 47; i > 0; i--){
        if(name[i] == ' ' || name[i] == 0) name[i] = 0;
        else break;
    }

    memcpy(str, name, 49);
}
