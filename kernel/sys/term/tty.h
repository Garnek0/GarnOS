/*  
*   File: tty.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef TTY_H
#define TTY_H

#include <garn/types.h>
#include <garn/fal/vnode.h>
#include <garn/fal/vfs.h>

extern vnode_t* tty;

void tty_init();

#endif //TTY_H
