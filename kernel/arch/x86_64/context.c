#include <arch/arch-internals.h>
#include <process/thread/thread.h>
#include <process/sched/sched.h>

void arch_store_context(stack_frame_t* regs){
    thread_t* currentThread = sched_get_current_thread();

    currentThread->regs = *regs;

	asm volatile("clts; fxsave (%0)" :: "r"(&currentThread->fpRegs));
}

void arch_restore_context(stack_frame_t* regs){
    thread_t* currentThread = sched_get_current_thread();

    *regs = currentThread->regs;

	asm volatile("fxrstor (%0)" :: "r"(&currentThread->fpRegs));

    wrmsr(0xC0000100, currentThread->tsp);
}

void arch_set_tsp(uintptr_t pointer, stack_frame_t* regs){
    thread_t* currentThread = sched_get_current_thread();

    currentThread->tsp = (uint64_t)pointer;
    wrmsr(0xC0000100, (uint64_t)pointer);
}
