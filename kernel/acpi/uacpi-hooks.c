#include <uacpi/kernel_api.h>
#include <uacpi/sleep.h>
#include <uacpi/types.h>
#include <garn/kernel.h>
#include <garn/arch.h>
#include <garn/mm.h>
#include <garn/hw/pci.h>
#include <garn/timer.h>
#include <garn/irq.h>

uacpi_status uacpi_kernel_raw_memory_read(uacpi_phys_addr address, uacpi_u8 byte_width, uacpi_u64 *out_value){
    switch(byte_width){
        case 1:
            *out_value = (uacpi_u64)*((uacpi_u8*)(address + (uacpi_u64)hhdmOffset));
            break;
        case 2:
            *out_value = (uacpi_u64)*((uacpi_u16*)(address + (uacpi_u64)hhdmOffset));
            break;
        case 4:
            *out_value = (uacpi_u64)*((uacpi_u32*)(address + (uacpi_u64)hhdmOffset));
            break;
        case 8:
            *out_value = (uacpi_u64)*((uacpi_u64*)(address + (uacpi_u64)hhdmOffset));
            break;
        default:
            break;
    }
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_raw_memory_write(uacpi_phys_addr address, uacpi_u8 byte_width, uacpi_u64 in_value){
    *((uacpi_u64*)(address + (uacpi_u64)hhdmOffset)) = in_value;
    switch(byte_width){
        case 1:
            *((uacpi_u8*)(address + (uacpi_u64)hhdmOffset)) = (uacpi_u8)in_value;
            break;
        case 2:
            *((uacpi_u16*)(address + (uacpi_u64)hhdmOffset)) = (uacpi_u16)in_value;
            break;
        case 4:
            *((uacpi_u32*)(address + (uacpi_u64)hhdmOffset)) = (uacpi_u32)in_value;
            break;
        case 8:
            *((uacpi_u64*)(address + (uacpi_u64)hhdmOffset)) = in_value;
            break;
        default:
            break;
    }
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_raw_io_read(uacpi_io_addr address, uacpi_u8 byte_width, uacpi_u64 *out_value){
    switch(byte_width){
        case 1:
            *out_value = (uacpi_u64)arch_inb((uint16_t)address);
            break;
        case 2:
            *out_value = (uacpi_u64)arch_inw((uint16_t)address);
            break;
        case 4:
            *out_value = (uacpi_u64)arch_inl((uint16_t)address);
            break;
        default:
            break;
    }
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_raw_io_write(uacpi_io_addr address, uacpi_u8 byte_width, uacpi_u64 in_value){
    switch(byte_width){
        case 1:
            arch_outb((uint16_t)address, (uint8_t)in_value);
            break;
        case 2:
            arch_outw((uint16_t)address, (uint16_t)in_value);
            break;
        case 4:
            arch_outl((uint16_t)address, (uint32_t)in_value);
            break;
        default:
            break;
    }
    return UACPI_STATUS_OK;
}

void *uacpi_kernel_alloc(uacpi_size size){
    return kmalloc((size_t)size);
}

void *uacpi_kernel_calloc(uacpi_size count, uacpi_size size){
    void* addr = kmalloc((size_t)(count*size));
    memset(addr, 0, (size_t)(count*size));
    return addr;
}

#ifndef UACPI_SIZED_FREES
void uacpi_kernel_free(void *mem){
    if(mem == NULL) return;
    kmfree(mem);
}
#else
void uacpi_kernel_free(void *mem, uacpi_size size_hint){
    kmfree(mem);
}
#endif

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len){
    vmm_map_range(vmm_get_kernel_pt(), (uint64_t)addr, (uint64_t)addr + hhdmOffset, len, (VMM_CACHE_DISABLE | VMM_RW | VMM_PRESENT));
    return (void*)((uint64_t)addr + hhdmOffset);
}

void uacpi_kernel_unmap(void *addr, uacpi_size len){
    vmm_unmap_range(vmm_get_kernel_pt(), (uint64_t)addr, len);
}

uacpi_status uacpi_kernel_pci_read(uacpi_pci_address *address, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 *value){
    pci_location_t location;
    location.bus = address->bus;
    location.dev = address->device;
    location.func = address->function;

    switch(byte_width){
        case 1:
            *value = (uacpi_u64)((uint8_t)(pci_config_read_word(location, offset) & 0xFF));
            break;
        case 2:
            *value = (uacpi_u64)(pci_config_read_word(location, offset));
            break;
        case 4:
            *value = (uacpi_u64)(pci_config_read_address(location, offset));
            break;
        default:
            break;
    }
    return UACPI_STATUS_OK;

    return UACPI_STATUS_UNIMPLEMENTED;

}

uacpi_status uacpi_kernel_pci_write(uacpi_pci_address *address, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 value){
    pci_location_t location;
    location.bus = address->bus;
    location.dev = address->device;
    location.func = address->function;

    switch(byte_width){
        case 1:
            uint16_t pciWord = pci_config_read_word(location, offset);
            pciWord = (pciWord & 0xFF00) | (value & 0xFF);
            pci_config_write_word(location, offset, pciWord); 
            break;
        case 2:
            pci_config_write_word(location, offset, (uint16_t)value);
            break;
        case 4:
            pci_config_write_word(location, offset, (uint16_t)(value & 0xFFFF));
            value >>= 16;
            pci_config_write_word(location, offset+2, (uint16_t)(value & 0xFFFF));
            break;
        default:
            break;
    }
    return UACPI_STATUS_OK;
}


uacpi_u64 uacpi_kernel_get_ticks(void){
    return (uacpi_u64)timer_get_ticks()*10000;
}

void uacpi_kernel_stall(uacpi_u8 usec){
    ksleep(1);
}

void uacpi_kernel_sleep(uacpi_u64 msec){
    ksleep(msec);
}

uacpi_handle uacpi_kernel_create_spinlock(void){
    uacpi_handle spinlock = (uacpi_handle)uacpi_kernel_calloc(1, sizeof(spinlock_t));
    return spinlock;
}

void uacpi_kernel_free_spinlock(uacpi_handle spinlock){
    uacpi_kernel_free(spinlock);
}

uacpi_cpu_flags uacpi_kernel_spinlock_lock(uacpi_handle spinlock){
    spinlock_t* lock = (spinlock_t*)spinlock;
    acquireLock(lock);
    return 0;
}

void uacpi_kernel_spinlock_unlock(uacpi_handle spinlock, uacpi_cpu_flags flags){
    spinlock_t* lock = (spinlock_t*)spinlock;
    releaseLock(lock);
}

uacpi_status uacpi_kernel_schedule_work(uacpi_work_type workType, uacpi_work_handler workHandler, uacpi_handle ctx){
    workHandler(ctx);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_wait_for_work_completion(void){
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_install_interrupt_handler(uacpi_u32 irq, uacpi_interrupt_handler handler, uacpi_handle ctx, uacpi_handle *out_irq_handle){
    irq_add_handler(irq, handler, 0);
    *out_irq_handle = (void*)irq;
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_uninstall_interrupt_handler(uacpi_interrupt_handler handler, uacpi_handle irq_handle){
    irq_remove_handler((uint8_t)irq_handle, handler);
    return UACPI_STATUS_OK;
}

uacpi_handle uacpi_kernel_create_mutex(void){
    return uacpi_kernel_create_spinlock();
}

void uacpi_kernel_free_mutex(uacpi_handle mutex){
    uacpi_kernel_free_spinlock(mutex);
}

uacpi_bool uacpi_kernel_acquire_mutex(uacpi_handle counter, uacpi_u16 timeout){
    spinlock_t* mutex = (spinlock_t*)counter;
    if(timeout == 0xFFFF){
        acquireLock(mutex);
        return UACPI_TRUE;
    } else {
        while(timeout){
            timeout--;
            if(*(spinlock_t*)counter == 0){
                acquireLock(mutex);
                return UACPI_TRUE;
            }
            ksleep(1);
        }
        return UACPI_FALSE;
    }
}

void uacpi_kernel_release_mutex(uacpi_handle mutex){
    releaseLock((spinlock_t*)mutex);
}

uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len, uacpi_handle *out_handle){
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_read(uacpi_handle port, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 *value){
    return uacpi_kernel_raw_io_read(offset, byte_width, value);
}

uacpi_status uacpi_kernel_io_write(uacpi_handle port, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 value){
    return uacpi_kernel_raw_io_write(offset, byte_width, value);
}

void uacpi_kernel_io_unmap(uacpi_handle handle){
    return;
}

uacpi_handle uacpi_kernel_create_event(void){
    return uacpi_kernel_create_spinlock();
}

void uacpi_kernel_free_event(uacpi_handle event){
    uacpi_kernel_free_spinlock(event);
}

uacpi_thread_id uacpi_kernel_get_thread_id(void){
    return NULL;
}

spinlock_t eventLock;

void uacpi_kernel_signal_event(uacpi_handle event){
    lock(eventLock, {
        *(spinlock_t*)event++;
    });
}

void uacpi_kernel_reset_event(uacpi_handle event){
    lock(eventLock, {
        *(spinlock_t*)event = 0;
    });
}

uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request *req){
    switch(req->type){
        case UACPI_FIRMWARE_REQUEST_TYPE_BREAKPOINT:
            break;
        case UACPI_FIRMWARE_REQUEST_TYPE_FATAL:
            panic("Fatal firmware request!", "uACPI");
            break;
        default:
            panic("Unhandled firmware request!", "uACPI");
            break;
    }
    return UACPI_STATUS_OK;
}

uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle counter, uacpi_u16 timeout){
    lock(eventLock, {
        if(timeout == 0xFFFF){
            while(*(spinlock_t*)counter == 0) pause();
            *(spinlock_t*)counter--;
            releaseLock(&eventLock);
            return UACPI_TRUE;
        } else {
            while(timeout){
                timeout--;
                if(*(spinlock_t*)counter > 0){
                    *(spinlock_t*)counter--;
                    releaseLock(&eventLock);
                    return UACPI_TRUE;
                }
                ksleep(1);
            }
            releaseLock(&eventLock);
            return UACPI_FALSE;
        }
    });
}

void uacpi_kernel_log(uacpi_log_level loglevel, const uacpi_char* str){
    switch(loglevel){
        case UACPI_LOG_INFO:
        case UACPI_LOG_DEBUG:
        case UACPI_LOG_TRACE:
            loglevel = KLOG_INFO;
            break;
        case UACPI_LOG_ERROR:
            loglevel = KLOG_FAILED;
            break;
        case UACPI_LOG_WARN:
            loglevel = KLOG_WARNING;
            break;
    }

    klog(str, loglevel, "uACPI");
}
