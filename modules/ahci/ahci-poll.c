#include "ahci.h"

bool ahci_wait_set(volatile uint32_t* reg, uint32_t bit, uint32_t ms){
    for(size_t i = 0; i < ms + 1; i++){
        if(*reg & bit) return true;
        ksleep(1);
    }
    return false;
}

bool ahci_wait_clear(volatile uint32_t* reg, uint32_t bit, uint32_t ms){
    for(size_t i = 0; i < ms + 1; i++){
        if(!(*reg & bit)) return true;
        ksleep(1);
    }
    return false;
}
