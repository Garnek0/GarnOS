#pragma once

typedef struct {
    char cmd[16];
    void (*function)();
} kcon_command_t;

void init_kcon();
void kcon_add_command(char* cmd, void* function);
