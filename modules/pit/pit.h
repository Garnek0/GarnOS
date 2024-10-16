#pragma once

#include <garn/types.h>

#define PIT_CHANNEL_0 0x40
#define PIT_CHANNEL_1 0x41
#define PIT_CHANNEL_2 0x42
#define PIT_MODE_OR_COMMAND 0x43

#define PIT_BASE_FREQUENCY 1193182

void pit_set_frequency(uint32_t freq);
uint64_t pit_get_ticks();
