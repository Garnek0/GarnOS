#pragma once

#include <garn/types.h>
#include <garn/irq.h>
#include <garn/uname.h>

int sys_uname(stack_frame_t* regs, utsname_t* buf);
