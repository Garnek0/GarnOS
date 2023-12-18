#include "compat.h"
#include <sys/panic.h>
#include <cpuid.h>

enum {
    CPUID_FEAT_EDX_FPU          = 1 << 0,   
    CPUID_FEAT_EDX_MSR          = 1 << 5,  
    CPUID_FEAT_EDX_PAE          = 1 << 6,   
    CPUID_FEAT_EDX_APIC         = 1 << 9,   
    CPUID_FEAT_EDX_NX           = 1 << 20,
    CPUID_FEAT_EDX_LM           = 1 << 29,
};

void compat_check(){
    uint32_t eax, ebx, ecx, edx;
    __get_cpuid(0x80000001, &eax, &ebx, &ecx, &edx);

    if(!(edx & CPUID_FEAT_EDX_LM)) panic("Unsupported CPU! (Long Mode Not Supported)"); //check Long Mode Supported
    if(!(edx & CPUID_FEAT_EDX_FPU)) panic("Unsupported CPU! (FPU Not Available)"); //check FPU availability
    if(!(edx & CPUID_FEAT_EDX_APIC)) panic("Unsupported CPU! (APIC Not Available)"); //check APIC availability
    if(!(edx & CPUID_FEAT_EDX_MSR)) panic("Unsupported CPU! (MSR Not Available)"); //check MSR availability
    if(!(edx & CPUID_FEAT_EDX_PAE)) panic("Unsupported CPU! (PAE Not Available)"); //check PAE availability
    if(!(edx & CPUID_FEAT_EDX_NX)) panic("Unsupported CPU! (NX Not Available)"); //check NX availability
}