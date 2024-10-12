#include <sys/dal/dal-internals.h>
#include <garn/dal/dal.h>
#include <garn/mm.h>
#include <garn/arch.h>

void devdetect(){
	acpidev_detect();

    pcidev_detect();
}
