/*  
*   File: module.h
*
*   Author: Garnek
*   
*   Description: Module Metadata struct definition & other module related stuff
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MODMGR_H
#define MODMGR_H

#include <garn/types.h>

typedef struct {
	char* name;
	void (*init)(void);
	void (*fini)(void);
} module_t;

#endif //MODMGR_H