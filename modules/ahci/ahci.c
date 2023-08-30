#include <module/module.h>
#include <kstdio.h>

void init(){
    return;
}

void fini(){
    return;
}

module_t metadata = {
    .name = "ahci",
    .init = init,
    .fini = fini
};