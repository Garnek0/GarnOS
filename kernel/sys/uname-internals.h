/*  
*   File: uname-internals.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef UNAME_INTERNALS_H
#define UNAME_INTERNALS_H

#include <garn/types.h>
#include <garn/irq.h>
#include <garn/uname.h>

int sys_uname(stack_frame_t* regs, utsname_t* buf);

#endif //UNAME_INTERNALS_H