/*  
*   File: uname.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef UNAME_H
#define UNAME_H

#include <types.h>
#include <cpu/interrupts/interrupts.h>

#define UNAME_SYS_NAME "Garn"
#define UNAME_DEFAULT_NODE_NAME "Host"
#define UNAME_MACHINE_X86_64 "x86_64"

typedef struct _utsname {
    char sysName[65];
	char nodeName[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
} utsname_t;

int sys_uname(stack_frame_t* regs, utsname_t* buf);

#endif //UNAME_H