/*  
*   File: process.c
*
*   Author: Garnek
*   
*   Description: Process Implementation
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "process.h"
#include <mem/mm-internals.h>
#include <process/thread/thread.h>
#include <arch/arch-internals.h>
#include <sys/term/tty.h>
#include <process/sched/sched.h>
#include <garn/input.h>
#include <exec/elf.h>
#include <garn/kstdio.h>
#include <garn/kerrno.h>
#include <garn/kernel.h>
#include <garn/arch.h>

#define PUSH(type,val,stack) do { \
	stack -= sizeof(type); \
	while(stack & (sizeof(type)-1)) stack--; \
	*((type*)stack) = (type)(val); \
} while(0)

#define PUSHSTR(s,stack) do { \
	ssize_t l = strlen(s); \
	do { \
		PUSH(char,s[l],stack); \
		l--; \
	} while (l>=0); \
} while (0)


process_t* processList;
process_t* processListLast;
process_t* initProc;

static int _process_gen_pid(){
    static int pid = 1;
    return pid++;
}

void process_create_init(){
    process_t* initProcess = kmalloc(sizeof(process_t));
    memset(initProcess, 0, sizeof(process_t));

    //Set PID and parent
    initProcess->pid = _process_gen_pid();
    initProcess->parent = NULL;
    initProcess->exitStatus = 0;
    initProcess->status = PROCESS_STATUS_RUNNING;

    //Create VA Space and allocate the main thread
    initProcess->pml4 = vaspace_new();

    thread_t* thread = kmalloc(sizeof(thread_t));
    thread->process = initProcess;
    thread->status = THREAD_STATUS_READY;

    initProcess->mainThread = thread;

    //No alignment is needed since addresses
    //returned by kmalloc() are already 16-byte aligned
    thread->kernelStackDeallocAddress = kmalloc(VMM_INIT_KERNEL_STACK_SIZE+1);
    thread->kernelStack = thread->kernelStackDeallocAddress + VMM_INIT_KERNEL_STACK_SIZE;
    thread->tsp = 0;

    vaspace_create_thread_user_stack(thread);

    //Set name
    initProcess->name = strdup("0:/bin/init.elf");

    //Create fd table
    initProcess->fdMax = PROCESS_INIT_FD-1;
    initProcess->fdTable = file_alloc_fd_table(PROCESS_INIT_FD);
    
    //Set rootDir and cwd
    initProcess->cwd = strdup("0:/");

    //Set tty FDs
    initProcess->fdTable[0].file = kbd;
    initProcess->fdTable[1].file = tty;
    initProcess->fdTable[2].file = tty;
    initProcess->fdTable[0].flags = tty->flags;
    initProcess->fdTable[1].flags = tty->flags;
    initProcess->fdTable[2].flags = tty->flags;
    tty->refCount+=2;
    kbd->refCount++;

    if(elf_exec_load(initProcess, "0:/bin/init.elf") != 0){
        panic("Could not load init!", "proc");
    }

    processList = processListLast = initProcess;
    initProcess->next = NULL;

    initProc = initProcess;

    vaspace_switch(initProcess->pml4);

    char* argvPtrs[1];
    PUSHSTR("./bin/init.elf", initProcess->mainThread->regs.rsp);
    argvPtrs[0] = (char*)initProcess->mainThread->regs.rsp;
    PUSH(uint64_t, 0, initProcess->mainThread->regs.rsp); // Aux vector null
    PUSH(uint64_t, 0, initProcess->mainThread->regs.rsp); // 0
    // envp should be between these
    PUSH(uint64_t, 0, initProcess->mainThread->regs.rsp); // 0
    PUSH(uint64_t, argvPtrs[0], initProcess->mainThread->regs.rsp); // init single argument
    PUSH(uint64_t, 1, initProcess->mainThread->regs.rsp); // init argc = 1

    klog("Spawned init process. Jumping into userspace. Bye!\n", KLOG_OK, "proc");

    sched_add_thread(initProcess->mainThread);

    arch_set_kernel_stack(0, (uint64_t)initProcess->mainThread->kernelStack);
    arch_usermode_enter((void*)initProcess->mainThread->regs.rip, (void*)initProcess->mainThread->regs.rsp);
}

void process_init(){
    process_create_init();
}

void process_terminate(process_t* process){
    for(process_t* proc = processList; proc != NULL; proc=proc->next){
        if(proc == process){
            klog("Terminating process '%s'...\n", KLOG_INFO, "proc", proc->name);
            proc->status = PROCESS_STATUS_ZOMBIE;
            sched_remove_thread(proc->mainThread); //TODO: remove all associated threads
            break;
        }
    }
}

void process_terminate_exception(process_t* process, stack_frame_t* regs, const char* message){
    //FIXME: MIGHT BREAK WITH MULTIPROCESSING
    kernel_screen_output_enable();
    kprintf("PID %d (%s): Process terminated due to exception. (%s)\n\n", process->pid, process->name, message);
    arch_dump_cpu_state(regs);
    kernel_screen_output_disable();
    process_terminate(process);

    arch_enable_interrupts();
    while(1) arch_no_op(); //Wait for reschedule
}

void process_free(process_t* process){
    process_t* prev = NULL;
    for(process_t* proc = processList; proc != NULL; proc=proc->next){
        if(proc == process){
            klog("Freeing process structure for '%s'...\n", KLOG_INFO, "proc", proc->name);
            if(proc == processListLast) processListLast = prev;
            if(proc == processList) processList = proc->next;
            if(prev) prev->next = proc->next;

            kmfree(proc->name); //name and other strings should be strduped
            kmfree(proc->cwd);
            for(size_t i = 0; i < proc->fdMax; i++){
                if(proc->fdTable[i].file) proc->fdTable[i].file->refCount--;
            }
            vaspace_destroy(proc->pml4);

            for(process_t* childProc = processList; childProc != NULL; childProc=childProc->next){
                if(childProc->parent == proc) childProc->parent = initProc;
            }

            kmfree(proc);

            return;
        }
        prev = proc;
    }
}

int sys_fork(stack_frame_t* regs){
    process_t* currentProcess = sched_get_current_process();
    process_t* newProcess = kmalloc(sizeof(process_t));
    memset(newProcess, 0, sizeof(process_t));

    newProcess->pid = _process_gen_pid();
    newProcess->parent = currentProcess;
    newProcess->name = kmalloc(strlen(currentProcess->name)+8);
    memcpy((void*)newProcess->name, currentProcess->name, strlen(currentProcess->name));
    memcpy((void*)&newProcess->name[strlen(currentProcess->name)], " (fork)", 8);
    newProcess->exitStatus = 0;
    newProcess->status = PROCESS_STATUS_RUNNING;

    newProcess->fdMax = currentProcess->fdMax;
    newProcess->fdTable = file_alloc_fd_table(newProcess->fdMax+1);

    newProcess->cwd = strdup(currentProcess->cwd);

    for(size_t i = 0; i < currentProcess->fdMax; i++){
        newProcess->fdTable[i] = currentProcess->fdTable[i];
        if(newProcess->fdTable[i].file) newProcess->fdTable[i].file->refCount++;
    }

    newProcess->mainThread = kmalloc(sizeof(thread_t));
    memset(newProcess->mainThread, 0, sizeof(thread_t));
    memcpy((void*)&newProcess->mainThread->regs, (void*)&currentProcess->mainThread->regs, sizeof(stack_frame_t));
    newProcess->mainThread->tsp = currentProcess->mainThread->tsp;

    newProcess->mainThread->kernelStackDeallocAddress = kmalloc(VMM_INIT_KERNEL_STACK_SIZE+1);
    newProcess->mainThread->kernelStack = newProcess->mainThread->kernelStackDeallocAddress + VMM_INIT_KERNEL_STACK_SIZE;
    newProcess->mainThread->status = THREAD_STATUS_READY;
    newProcess->mainThread->process = newProcess;

    newProcess->pml4 = vaspace_clone(currentProcess->pml4);

    processListLast->next = newProcess;
    processListLast = newProcess;

    klog("Forked Process from '%s'.\n", KLOG_OK, "proc", currentProcess->name);

    sched_add_thread(newProcess->mainThread);

    newProcess->mainThread->regs.rax = 0;
    return newProcess->pid;
}

__attribute__((noreturn))
void sys_exit(stack_frame_t* regs, int status){
    process_t* currentProcess = sched_get_current_process();

    currentProcess->exitStatus = status;
    process_terminate(currentProcess);

    while(1) arch_no_op(); //wait for reschedule

    __builtin_unreachable();
}

//TODO: Make this block the process

int sys_waitpid(stack_frame_t* regs, int64_t pid, int* status, int options){
    process_t* currentProcess = sched_get_current_process();

    //TODO: Implement this
    if(pid < -1) return -ENOSYS;
    if(pid == 0) return -ENOSYS;

    for(;;){
        bool done = false;
        for(process_t* childProc = processList; childProc != NULL; childProc=childProc->next){
            if(childProc == NULL || childProc->status != PROCESS_STATUS_ZOMBIE) continue;
            if(childProc->parent != currentProcess) continue;
            if(pid > 0 && childProc->pid != pid) continue;

            *status = childProc->exitStatus;
            process_free(childProc);
            done = true;
            break;
        }
        if(done) break;
    }

    return 0;
}

int sys_execve(stack_frame_t* regs, const char* path, const char* argv[], const char* envp[]){
    //TODO: destroy all threads but the main one.

    if(argv == NULL || envp == NULL) return -EFAULT;

    process_t* currentProcess = sched_get_current_process();

    klog("execveing Process '%s'...\n", KLOG_INFO, "proc", currentProcess->name);

    path = file_get_absolute_path(currentProcess->cwd, (char*)path);
    if(!path) return -kerrno;

    currentProcess->name = strdup(path);
    currentProcess->status = PROCESS_STATUS_RUNNING;

    //Check if 'path' is valid

    file_t* file = file_open((char*)path, O_RDONLY, 0);
    if(!file) return -kerrno;
    file_close(file);

    //Count and realloc args anv env vars

    size_t envc = 0, argc = 0;
    while(argv[argc] != NULL){
        argv[argc] = strdup(argv[argc]);
        argc++;
    }
    while(envp[envc] != NULL){
        envp[envc] = strdup(envp[envc]);
        envc++;   
    }

    //Reallocate argv and envp in kernel space

    char** newArgv = kmalloc((argc)*sizeof(char*));
    char** newEnvp = kmalloc((envc)*sizeof(char*));
    for(size_t i = 0; i < argc; i++) newArgv[i] = (char*)argv[i];
    for(size_t i = 0; i < envc; i++) newEnvp[i] = (char*)envp[i];

    vaspace_clear(currentProcess->pml4);

    arch_disable_interrupts(); //Stop interrupts from messing up the thread state

    vaspace_create_thread_user_stack(currentProcess->mainThread);

    //Push argv and envp strings

    for(size_t i = 0; i < argc; i++){
        PUSHSTR(newArgv[i], currentProcess->mainThread->regs.rsp);
        kmfree(newArgv[i]);
        newArgv[i] = (char*)currentProcess->mainThread->regs.rsp;
    }

    for(size_t i = 0; i < envc; i++){
        PUSHSTR(newEnvp[i], currentProcess->mainThread->regs.rsp);
        kmfree(newEnvp[i]);
        newEnvp[i] = (char*)currentProcess->mainThread->regs.rsp;
    }

    //Create initial process stack
    PUSH(uint64_t, (uint64_t)0, currentProcess->mainThread->regs.rsp); //Aux vector NULL
    PUSH(uint64_t, (uint64_t)0, currentProcess->mainThread->regs.rsp); //envp NULL
    for(ssize_t i = envc-1; i >= 0; i--) PUSH(uint64_t, newEnvp[i], currentProcess->mainThread->regs.rsp); //envp
    PUSH(uint64_t, (uint64_t)0, currentProcess->mainThread->regs.rsp); //argv NULL
    for(ssize_t i = argc-1; i >= 0; i--) PUSH(uint64_t, newArgv[i], currentProcess->mainThread->regs.rsp); //argv
    PUSH(uint64_t, argc, currentProcess->mainThread->regs.rsp); //argc

    //Load image and jump to entry point

    if(elf_exec_load(currentProcess, (char*)path) != 0){
        return -ENOENT;
    }

    vaspace_switch(currentProcess->pml4);

    kmfree((void*)path);
    kmfree(newArgv);
    kmfree(newEnvp);

    *regs = currentProcess->mainThread->regs;

    klog("execve'd Process '%s'. Jumping to entry point...\n", KLOG_OK, "proc", currentProcess->name);

    arch_set_kernel_stack(0, (uint64_t)currentProcess->mainThread->kernelStack);
    arch_usermode_enter((void*)currentProcess->mainThread->regs.rip, (void*)currentProcess->mainThread->regs.rsp);

    __builtin_unreachable();

    return 0;
}