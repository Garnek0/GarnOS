/*  
*   File: process.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef PROCESS_H
#define PROCESS_H

#define PROCESS_INIT_FD 128
#define PROCESS_MAX_FD 2048

#include <garn/types.h>
#include <process/thread/thread.h>
#include <sys/fal/fal-internals.h>
#include <garn/fal/file.h>
#include <garn/mm.h>

#define PROCESS_STATUS_RUNNING 0
#define PROCESS_STATUS_ZOMBIE 1

typedef struct _process {
    char* name;
    int pid;
    int exitStatus;
    uint8_t status;

    struct _page_table* pt;

    fd_t* fdTable;
    size_t fdMax;

    char* cwd;

    struct _thread* mainThread;
    struct _process* parent;

    struct _process* next;
} process_t;

void process_init();
void process_terminate(process_t* process);
void process_terminate_exception(process_t* process, stack_frame_t* regs,  const char* message);
void process_free(process_t* process);

int sys_fork(stack_frame_t* regs);
void sys_exit(stack_frame_t* regs, int status);
int sys_waitpid(stack_frame_t* regs, int64_t pid, int* status, int options);
int sys_execve(stack_frame_t* regs, const char* path, const char* argv[], const char* envp[]);

#endif //PROCESS_H