/*  
*   File: syscall.c
*
*   Author: Garnek
*   
*   Description: System Calls
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "syscall_internals.h"

#include <sys/uname-internals.h>
#include <sys/dal/dal-internals.h>
#include <garn/dal/dal.h>
#include <garn/fal/vnode.h>
#include <garn/fal/vfs.h>
#include <mem/mm-internals.h>
#include <process/process.h>
#include <process/sched/sched.h>
#include <sys/fal/fal-internals.h>

#include <garn/irq.h>
#include <arch/arch-internals.h>
#include <garn/kstdio.h>

#include <garn/syscall.h>

void* syscallTable[SYSCALL_MAX];
void* gsysSyscallTable[SYSCALL_MAX];

void syscall_register(size_t num, void* addr){
    syscallTable[num] = addr;
}

void gsys_syscall_register(size_t num, void* addr){
    gsysSyscallTable[num] = addr;
}

void syscall_init(){
    // POSIX & Linux Syscalls

    syscall_register(0, sys_read);
    syscall_register(1, sys_write);
    syscall_register(2, sys_open);
    syscall_register(3, sys_close);
    syscall_register(7, sys_waitpid);
    syscall_register(9, sys_mmap);
    syscall_register(11, sys_munmap);
    syscall_register(57, sys_fork);
    syscall_register(59, sys_execve);
    syscall_register(60, sys_exit);
    syscall_register(63, sys_uname);
    syscall_register(78, sys_getdents);
    syscall_register(79, sys_getcwd);
    syscall_register(80, sys_chdir);
    syscall_register(162, sys_sync);

    // Garn Syscalls (> 0x80000000)

    gsys_syscall_register(0x80000000 - 0x80000000, sys_set_tsp);

    arch_syscall_init();

    klog("Syscalls initialised.\n", KLOG_OK, "syscall");
}

void syscall_handler(stack_frame_t* regs){
    thread_t* callerThread = sched_get_current_thread();

    callerThread->regs = *regs;

    if(arch_syscall_number(regs) < 0x80000000){
        int (*syscall)(stack_frame_t* regs,
        long arg0, 
        long arg1, 
        long arg2, 
        long arg3, 
        long arg4, 
        long arg5) = (int(*)(stack_frame_t* regs, long arg0, long arg1, long arg2, long arg3, long arg4, long arg5))syscallTable[arch_syscall_number(regs)];

        arch_enable_interrupts(); //enable interrupts
        
        long result;

        if(syscall) result = syscall(regs, arch_syscall_arg0(regs), arch_syscall_arg1(regs), arch_syscall_arg2(regs), arch_syscall_arg3(regs), arch_syscall_arg4(regs), arch_syscall_arg5(regs));

        arch_syscall_return(regs, result);
    } else {
        int (*syscall)(stack_frame_t* regs,
        long arg0, 
        long arg1, 
        long arg2, 
        long arg3, 
        long arg4, 
        long arg5) = (int(*)(stack_frame_t* regs, long arg0, long arg1, long arg2, long arg3, long arg4, long arg5))gsysSyscallTable[arch_syscall_number(regs) - 0x80000000];

        arch_enable_interrupts(); //enable interrupts
        
        long result;

        if(syscall) result = syscall(regs, arch_syscall_arg0(regs), arch_syscall_arg1(regs), arch_syscall_arg2(regs), arch_syscall_arg3(regs), arch_syscall_arg4(regs), arch_syscall_arg5(regs));

        arch_syscall_return(regs, result);        
    }
}
