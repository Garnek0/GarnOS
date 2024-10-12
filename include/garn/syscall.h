#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_MAX 255
#define SYSCALL_GSYS_MAX 255

#include <garn/types.h>

void syscall_register(size_t num, void* addr);
void gsys_syscall_register(size_t num, void* addr);

#endif //SYSCALL_H
