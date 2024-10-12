#pragma once

#include <garn/types.h>
#include <garn/irq.h>
#include <garn/kstdio.h>

void panic(const char* str, const char* component, ...);
void panic_exception(const char* str, stack_frame_t* regs, ...);
