#include "acpi-internals.h"
#include <uacpi/sleep.h>
#include <garn/panic.h>
#include <sys/bootloader.h>
#include <garn/power.h>

#include <garn/kstdio.h>
#include <garn/config.h>

void acpi_init(){
    uacpi_phys_addr rsdpAddr = (uacpi_phys_addr)bl_get_rsdp_address() - bl_get_hhdm_offset();

    uacpi_init_params init_params = {
        .rsdp = rsdpAddr,
        .log_level = UACPI_LOG_INFO,
        .flags = 0,
    };

    uacpi_status ret = uacpi_initialize(&init_params);
    if (uacpi_unlikely_error(ret)) {
        panic("uacpi_initialize error: %s. Your system may not support ACPI.", "uACPI", uacpi_status_to_string(ret));
    }

    ret = uacpi_namespace_load();
    if (uacpi_unlikely_error(ret)) {
        panic("uacpi_namespace_load error: %s. Your system may not support ACPI.", "uACPI", uacpi_status_to_string(ret));
    }

    ret = uacpi_namespace_initialize();
    if (uacpi_unlikely_error(ret)) {
        panic("uacpi_namespace_initialize error: %s. Your system's ACPI implementation may be quirky.", "uACPI", uacpi_status_to_string(ret));
    }

    ret = uacpi_finalize_gpe_initialization();
    if (uacpi_unlikely_error(ret)) {
        panic("uACPI GPE initialization error: %s. Your system's ACPI implementation may be quirky.", "uACPI", uacpi_status_to_string(ret));
    }

    power_set_reboot(uacpi_reboot);
    power_set_shutdown(uacpi_shutdown);
}
