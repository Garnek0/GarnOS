/*  
*   File: miscdev.c
*
*   Author: Garnek
*   
*   Description: Misc devices such as a PS/2 controller
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <sys/dal/dal-internals.h>
#include <garn/dal/dal.h>
#include <garn/mm.h>
#include <garn/arch.h>
#include <garn/config.h>

void devdetect(){

	acpidev_detect();

#ifdef CONFIG_INCLUDE_PCI_DRIVER

    pcidev_detect();

#endif //CONFIG_INCLUDE_PCI_DRIVER
}
