/*  
*   File: dal-internals.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef DAL_INTERNALS_H
#define DAL_INTERNALS_H

#include <garn/types.h>
#include <module/module-internals.h>
#include <garn/kstdio.h>

#include <garn/spinlock.h>
#include <garn/panic.h>
#include <garn/dal/dal.h>
#include <garn/dal/device-types.h>
#include <garn/dal/bcache.h>

typedef struct {
    bcache_buf_t* head;
    bcache_buf_t* tail;
    bcache_buf_t buf[BCACHE_BUF_COUNT];

    spinlock_t spinlock;
} bcache_t;

//device

void device_init();

//driver

void driver_init();

//pcidev

void pcidev_detect();

//devdetect

void devdetect();

//checksysfs

bool checksysfs_check();

//bcache

void bcache_init();

//DAL

void dal_init();

int sys_sync(stack_frame_t* regs);

#endif //DAL_INTERNALS_H