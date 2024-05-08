/*  
*   File: compat.c
*
*   Author: Garnek
*   
*   Description: Compatibility Checks
*/
// SPDX-License-Identifier: BSD-2-Clause

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
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    __get_cpuid(0x1, &eax, &ebx, &ecx, &edx);

    if(!(edx & CPUID_FEAT_EDX_FPU)) panic("Unsupported CPU! (FPU Not Available)", "compat"); //check FPU availability
    else klog("System supports x87 FPU.\n", KLOG_OK, "compat");
    if(!(edx & CPUID_FEAT_EDX_APIC)) panic("Unsupported CPU! (APIC Not Available)", "compat"); //check APIC availability
    else klog("System supports APIC.\n", KLOG_OK, "compat");
    if(!(edx & CPUID_FEAT_EDX_MSR)) panic("Unsupported CPU! (MSR Not Available)", "compat"); //check MSR availability
    else klog("System supports MSR.\n", KLOG_OK, "compat");
    if(!(edx & CPUID_FEAT_EDX_PAE)) panic("Unsupported CPU! (PAE Not Supported)", "compat"); //check PAE support
    else klog("System supports PAE.\n", KLOG_OK, "compat");

    __get_cpuid(0x80000001, &eax, &ebx, &ecx, &edx);

    if(!(edx & CPUID_FEAT_EDX_NX)) panic("Unsupported CPU! (NX Not Supported)", "compat"); //check NX support
    else klog("System supports NX bit.\n", KLOG_OK, "compat");
}