/*  
*   File: dal.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef DAL_H
#define DAL_H

#include <types.h>
#include <module/module.h>
#include <kstdio.h>

#include "dev/device.h"
#include "dev/driver.h"
#include "stor/drive.h"
#include "stor/part/part.h"
#include "stor/bcache.h"
#include "checksysfs.h"
#include <cpu/multiproc/multiproc.h>

#include <sys/dal/dev/pcidev.h>
#include <sys/dal/dev/miscdev.h>
#include <sys/panic.h>

void dal_init();

#endif //DAL_H