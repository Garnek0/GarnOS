#ifndef THREAD_H
#define THREAD_H

#define THREAD_STATUS_READY 0
#define THREAD_STATUS_RUNNING 1
#define THREAD_STATUS_BLOCKED 2
#define THREAD_STATUS_DESTROYED 3

#include <garn/types.h>
#include <garn/irq.h>
#include <process/process.h>

struct _thread;

typedef struct _thread {
    int status;

    stack_frame_t regs;
    uint64_t tsp; //thread-self-pointer
	
	uint8_t fpRegs[512] __attribute__((aligned(16)));

    void* kernelStack;
    void* kernelStackDeallocAddress;

    struct _process* process;
} thread_t;

#endif //THREAD_H
