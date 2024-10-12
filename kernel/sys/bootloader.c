#include "bootloader.h"

static volatile struct limine_kernel_file_request kernel_file_request = {
    .id = LIMINE_KERNEL_FILE_REQUEST,
    .revision = 0
};

static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
    .flags = 0
};

static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

void* bl_get_kernel_file_address(){
    return kernel_file_request.response->kernel_file->address;
}

uint64_t bl_get_kernel_file_size(){
    return kernel_file_request.response->kernel_file->size;
}

uint64_t* bl_get_gpt_system_disk_uuid(){
    return (uint64_t*)&kernel_file_request.response->kernel_file->gpt_disk_uuid;
}

uint64_t* bl_get_gpt_system_partition_uuid(){
    return (uint64_t*)&kernel_file_request.response->kernel_file->gpt_part_uuid;
}

uint64_t* bl_get_mbr_system_disk_id(){
    return (uint64_t*)&kernel_file_request.response->kernel_file->mbr_disk_id;
}

uint64_t bl_get_kernel_phys_base(){
    return kernel_address_request.response->physical_base;
}

uint64_t bl_get_kernel_virt_base(){
    return kernel_address_request.response->virtual_base;
}

uint64_t bl_get_hhdm_offset(){
    return hhdm_request.response->offset;
}

size_t bl_get_cpu_count(){
    return smp_request.response->cpu_count;
}

uint32_t bl_get_bsp_lapic_id(){
    return smp_request.response->bsp_lapic_id;
}

struct limine_smp_info** bl_get_cpu_info_array(){
    return smp_request.response->cpus;
}

struct limine_smp_info* bl_get_cpu_info(size_t index){
    return smp_request.response->cpus[index];
}

uint8_t bl_is_x2apic(){
    return smp_request.response->flags;
}

void* bl_get_rsdp_address(){
    return rsdp_request.response->address;
}
