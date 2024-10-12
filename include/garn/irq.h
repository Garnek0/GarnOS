#ifndef IRQ_H
#define IRQ_H

#include <garn/types.h>

#define IRQ_MAX 256

#define IRQ_SHARED 1

int irq_add_handler(uint8_t irq, void* handler, uint32_t flags);
void irq_remove_handler(uint8_t irq, void* handler);

#ifdef __x86_64__

//x86_64 stack frame
typedef struct {
   uint64_t ds;
   uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
   uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
   uint64_t intn, errCode;
   uint64_t rip, cs, rflags, rsp, ss;
} stack_frame_t;

#elif ARCH_DUMMY

;

#endif

#endif //IRQ_H
