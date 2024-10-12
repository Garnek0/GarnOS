#ifndef MODMGR_H
#define MODMGR_H

#include <garn/types.h>

typedef struct {
	const char* name;
	void (*init)(void);
	void (*fini)(void);
} module_t;

#endif //MODMGR_H
