/*  
*   File: compat.c
*
*   Author: Garnek
*   
*   Description: x86 Compatibility Checks
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <garn/panic.h>
#include <arch/arch-internals.h>
#include <cpuid.h>

enum {
    CPUID_FEAT_EDX_FPU          = 1 << 0,   
    CPUID_FEAT_EDX_MSR          = 1 << 5,  
    CPUID_FEAT_EDX_PAE          = 1 << 6,   
    CPUID_FEAT_EDX_APIC         = 1 << 9,   
    CPUID_FEAT_EDX_NX           = 1 << 20,
    CPUID_FEAT_EDX_LM           = 1 << 29,
};

void arch_compat_checks(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    __get_cpuid(0x1, &eax, &ebx, &ecx, &edx);

    if(!(edx & CPUID_FEAT_EDX_FPU)) panic("Unsupported CPU! (x87 FPU Not Available)", "compat"); //check FPU availability
    else klog("System supports x87 FPU.\n", KLOG_OK, "compat");
    if(!(edx & CPUID_FEAT_EDX_APIC)) panic("Unsupported CPU! (APIC Not Available)", "compat"); //check APIC availability
    else klog("System supports APIC.\n", KLOG_OK, "compat");
    if(!(edx & CPUID_FEAT_EDX_MSR)) panic("Unsupported CPU! (MSR Not Available)", "compat"); //check MSR availability
    else klog("System supports MSR.\n", KLOG_OK, "compat");
}