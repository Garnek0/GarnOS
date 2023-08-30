#include <module/module.h>
#include <kstdio.h>

void init(){
    return;
}

void fini(){
    return;
}

module_t metadata = {
    .name = "ide",
    .init = init,
    .fini = fini
};