#include <sys/dal/dal-internals.h>
#include <garn/panic.h>

void dal_init(){
    device_init(); //Initialise device manager

    klog("DAL Initialised\n", KLOG_OK, "DAL");

    bcache_init(); //Initialise buffer cache

    smp_init(); //Initialize CPUs

    driver_init(); //Initialise device driver manager
    module_init(); //Initialise module manager

    devdetect(); //Detect devices
}
