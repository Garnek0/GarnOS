/*  
*   File: user.c
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <arch/x86/user/user.h>
#include <arch/arch-internals.h>

void arch_usermode_enter(uintptr_t entry, uintptr_t stack){
    user_jump((void*)entry, (void*)stack);
}
