#ifndef KCONDEMO_H
#define KCONDEMO_H

typedef struct {
    char cmd[16];
    void (*function)();
} kcon_command_t;

void init_kcon();
void kcon_add_command(char* cmd, void* function);

#endif //KCONDEMO_H
