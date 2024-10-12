#pragma once

#include <garn/types.h>

typedef struct {
	const char* name;
	void (*init)(void);
	void (*fini)(void);
} module_t;
