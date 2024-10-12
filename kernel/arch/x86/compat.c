/*  
*   File: compat.c
*
*   Author: Garnek
*   
*   Description: x86 Compatibility Checks
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <garn/panic.h>
#include <garn/config.h>
#include <arch/arch-internals.h>
#include <cpuid.h>

enum {
    CPUID_FEAT_EDX_FPU          = 1 << 0,   
    CPUID_FEAT_EDX_MSR          = 1 << 5,  
    CPUID_FEAT_EDX_APIC         = 1 << 9,   
	CPUID_FEAT_EDX_FXSR			= 1 << 24,
	CPUID_FEAT_EDX_SSE          = 1 << 25,
	CPUID_FEAT_EDX_SSE2			= 1 << 26,
};

void arch_compat_checks(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    __get_cpuid(0x1, &eax, &ebx, &ecx, &edx);

    if(!(edx & CPUID_FEAT_EDX_FPU)) klog("CPU doesn't support x87 FPU", KLOG_WARNING, "compat"); //check FPU support
    else klog("CPU supports x87 FPU.\n", KLOG_OK, "compat");

    if(!(edx & CPUID_FEAT_EDX_APIC)) panic("Unsupported CPU! (APIC Not Supported)", "compat"); //check APIC support
    else klog("CPU supports APIC.\n", KLOG_OK, "compat");

    if(!(edx & CPUID_FEAT_EDX_MSR)) panic("Unsupported CPU! (MSR Not Supported)", "compat"); //check MSR support
    else klog("CPU supports MSR.\n", KLOG_OK, "compat");

	if(!(edx & CPUID_FEAT_EDX_FXSR)) panic("Unsupported CPU! (FXSR Not Supported)", "compat"); //check FXSR support
    else klog("CPU supports FXSR.\n", KLOG_OK, "compat");

	if(!(edx & CPUID_FEAT_EDX_SSE)) panic("Unsupported CPU! (SSE Not Supported)", "compat"); //check SSE support (mandatory for 64-bit)
    else klog("CPU supports SSE.\n", KLOG_OK, "compat");

	if(!(edx & CPUID_FEAT_EDX_SSE2)) panic("Unsupported CPU! (SSE2 Not Supported)", "compat"); //check SSE2 support (mandatory for 64-bit)
    else klog("CPU supports SSE2.\n", KLOG_OK, "compat");
}
