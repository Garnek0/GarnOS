/*  
*   File: kcon.c
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef KCONDEMO_H
#define KCONDEMO_H

typedef struct _kcon_command {
    char cmd[16];
    void (*function)();
    struct _kcon_command* next;
} kcon_command_t;

void init_kcon();
void kcon_add_command(char* cmd, void* function);

#endif //KCONDEMO_H