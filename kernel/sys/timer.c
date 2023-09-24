#include "timer.h"
#include <hw/pit/pit.h>

//for now this is just a wrapper for the pit function
void ksleep(uint64_t ms){
    pit_sleep(ms);
}