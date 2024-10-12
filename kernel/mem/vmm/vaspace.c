#include <mem/mm-internals.h>
#include <sys/bootloader.h>
#include <process/sched/sched.h>
#include <garn/kerrno.h>
#include <garn/kstdio.h>
#include <arch/arch-internals.h>
#include <garn/config.h>

void vaspace_switch(page_table_t* pt){
    uint64_t hhdm = bl_get_hhdm_offset();
    uint64_t kernelVirtBase = bl_get_kernel_virt_base();
    if((uint64_t)pt > kernelVirtBase) pt = (page_table_t*)((uint64_t)pt - kernelVirtBase);
    else if((uint64_t)pt > hhdm) pt = (page_table_t*)((uint64_t)pt - hhdm);
    arch_vaspace_switch(pt);
}

page_table_t* vaspace_new(){
    page_table_t* pt = arch_vaspace_new();
    klog("New Address Space created. Page tables at 0x%x.\n", KLOG_OK, "VMM", (void*)pt);
    return pt;
}

void vaspace_destroy(page_table_t* pt){
    if(!pt) return;
    arch_vaspace_destroy(pt);
    klog("Page tables at 0x%x destroyed.\n", KLOG_OK, "VMM", (void*)pt);
}

void vaspace_clear(page_table_t* pt){
    if(!pt) return;
    arch_vaspace_clear(pt);
    klog("Page tables at 0x%x cleared.\n", KLOG_INFO, "VMM", (void*)pt);
}

page_table_t* vaspace_clone(page_table_t* toClone){
    page_table_t* clone = arch_vaspace_clone(toClone);
    klog("Page tables at 0x%x cloned.\n", KLOG_OK, "VMM", (void*)toClone);
    return clone;
}

void vaspace_create_thread_user_stack(thread_t* thread){
    //FIXME:
    //FIXME:
    //FIXME: VERY IMPORTANT! This will blow up when creating more than 1 thread/process.
#ifdef __x86_64__

    thread->regs.rsp = (uint64_t)vaspace_create_area(thread->process->pt, (VMM_USER_END - VMM_INIT_USER_STACK_SIZE),
                                        VMM_INIT_USER_STACK_SIZE, VMM_PRESENT | VMM_RW | VMM_USER | VMM_EXEC_DISABLE) + ((VMM_INIT_USER_STACK_SIZE - 1) - 15);
                            
#elif ARCH_DUMMY

;

#endif 
}

void* vaspace_create_area(page_table_t* pml4, uint64_t virtAddr, size_t size, uint32_t flags){
    if(size%PAGE_SIZE != 0) size = ALIGN_UP(size, PAGE_SIZE);

    void* physAddr;

    for(size_t i = 0; i < size; i+=PAGE_SIZE){
        physAddr = pmm_allocate(1);
        vmm_map(pml4, (uint64_t)physAddr, (uint64_t)virtAddr+i, flags);
    }

    return (void*)virtAddr;
}

void* sys_mmap(stack_frame_t* regs, void* addr, size_t length, int prot, int flags, int fd, uint64_t offset){
    if(length == 0) return (void*)-EINVAL;

    if((uint64_t)addr > VMM_USER_END) return (void*)-ENOMEM;

    //We dont support shared mappings yet
    if(flags & MAP_SHARED) return (void*)-EINVAL;
    if(!(flags & MAP_PRIVATE)) return (void*)-EINVAL;

    //We also dont support file mappings yet
    if(!(flags & MAP_ANONYMOUS)) return (void*)-EINVAL;

    process_t* currentProcess = sched_get_current_process();

    if(addr != NULL && (uint64_t)addr%PAGE_SIZE != 0) ALIGN_UP(addr, PAGE_SIZE);
    if(addr == NULL) addr = (void*)0x1000;

    if(length%PAGE_SIZE != 0) ALIGN_UP(length, PAGE_SIZE);
    length /= PAGE_SIZE;

    size_t npages = 0;

    uint64_t userEnd = VMM_USER_END;
    if(flags & MAP_32BIT) userEnd = 0x80000000;

    for(uint64_t i = (uint64_t)addr; i < userEnd; i+=PAGE_SIZE){
        
        if(vmm_is_page_free(currentProcess->pt, i)){
            goto found_addr;
        } else {
            npages = 0;
            continue;
        }

found_addr:
        npages++;
        if(npages == 1) addr = (void*)i;
        if(npages == length){
            uint64_t phys;
            uint64_t vmmflags = 0;

            vmmflags = (VMM_USER | VMM_PRESENT | VMM_EXEC_DISABLE);

            //if(prot & PROT_READ);
            if(prot & PROT_WRITE) vmmflags |= VMM_RW;
            if(prot & PROT_EXEC) vmmflags &= ~(VMM_EXEC_DISABLE);
            
            for(size_t j = 0; j < length; j++){
                phys = (uint64_t)pmm_allocate(1);
                vmm_map(currentProcess->pt, phys, (uint64_t)addr+(j*PAGE_SIZE), vmmflags);
            }
            return addr;
        } else continue;
    }

    return (void*)-ENOMEM;
}

int sys_munmap(stack_frame_t* regs, void* addr, size_t length){
    if(length == 0) return -EINVAL;
    if((uint64_t)addr > VMM_USER_END || (uint64_t)addr%PAGE_SIZE != 0) return -EINVAL;

    if(length%PAGE_SIZE != 0) ALIGN_UP(length, PAGE_SIZE);
    length /= PAGE_SIZE;

    process_t* currentProcess = sched_get_current_process();

    uint64_t physAddr;

    for(size_t i = 0; i < length; i++){
        physAddr = vmm_virt_to_phys(currentProcess->pt, (uint64_t)addr);

        if(physAddr == 0) return -EINVAL;
        else {
            pmm_free((void*)physAddr, 1);
            vmm_unmap(currentProcess->pt, (uint64_t)addr);
        }

        addr += PAGE_SIZE;
    }

    return -ENOMEM;
}
