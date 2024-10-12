#pragma once

#include <garn/types.h>

uint64_t rdmsr(uint64_t msr);
void wrmsr(uint64_t msr, uint64_t data);
