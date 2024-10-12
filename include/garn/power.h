#ifndef POWER_H
#define POWER_H

#include <garn/types.h>

typedef struct _power {
    int (*shutdown)();
    int (*reboot)();
    int (*suspend)();
} power_t;

int power_shutdown();
int power_reboot();
int power_suspend();

void power_set_shutdown(void* func);
void power_set_reboot(void* func);
void power_set_suspend(void* func);

#endif //POWER_H
