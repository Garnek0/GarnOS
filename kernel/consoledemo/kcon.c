/*  
*   File: kcon.c
*
*   Author: Garnek
*   
*   Description: Demo test console running in kernel mode (kcon)
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "kcon.h"
#include <garn/kstdio.h>
#include <garn/mm.h>
#include <garn/kernel.h>
#include <garn/time.h>
#include <garn/term/term.h>
#include <garn/dal/dal.h>
#include <garn/ds/list.h>
#include <garn/fal/vfs.h>
#include <garn/power.h>

list_t* commandList;
bool exit = false;

static void console_help(){
    kprintf("commands:\n"
            "\n"
            "help       - show this text\n"
            "clear      - clear screen\n"
            "ver        - show kernel version\n"
            "mm         - show memory and mm info\n"
            "timedate   - show RTC Time and Date\n"
            "dev        - list all devices\n"
            "cpu        - show all detected processors\n"
            "fs         - list mounted filesystems\n"
            "drives     - list all drives\n"
            "user       - enter userspace\n"
            "shutdown   - shutdown the system\n"
            "reboot     - perform a reboot\n");
}

static void console_clear(){
    term_clear();
}

static void console_mm(){
    size_t usablePages = pmm_get_usable_pages_count();
    size_t usedPages = pmm_get_used_pages_count();
    size_t freePages = pmm_get_free_pages_count();
    kprintf("%dMiB/%dMiB used\n"
            "kheap size: %dMiB\n", (usedPages*PAGE_SIZE/1024/1024), (usablePages*PAGE_SIZE/1024/1024), (kheap_get_size()/1024/1024));
}

static void console_ver(){
    kprintf(KERNEL_VER"\n");
}

static void console_timedate(){
    systime_t time = time_get();

    kprintf("%d/%d/%u\n", time.month, time.dayOfMonth, time.year);
    kprintf("%d:", time.hours);
    if(time.minutes < 10) kprintf("0");
    kprintf("%d:", time.minutes);
    if(time.seconds < 10) kprintf("0");
    kprintf("%d\n", time.seconds);
}

static void console_dev(){
    device_t device;
    for(size_t i = 0; i < device_get_device_count(); i++){
        device = device_get_device(i);
        kprintf("Device: %s\nDriver: ", device.name);
        if(!device.node){
            kprintf("No Device Driver Loaded\n\n");
            continue;
        }
        kprintf("%s\n\n", device.node->path);
    }
}

static void console_cpu(){
    device_t device;
    for(size_t i = 0; i < device_get_device_count(); i++){
        device = device_get_device(i);
        if(device.type == DEVICE_TYPE_PROCESSOR){
            kprintf("%s\n", device.name);
        }
    }
}

static void console_fs(){
	for(vfs_t* i = vfs_get_root(); i; i = i->next){
		 kprintf("FID=%d: %s - Size: %dKiB (%d Bytes)\n", i->fid, i->name, i->size/1024, i->size);
	}
}

static void console_drives(){
    for(drive_t* i = drive_get_list(); i; i = i->next){
        kprintf("driveID %d: %s\n", i->driveid, i->name);
        if(i->type == DRIVE_TYPE_OPTICAL) {
            kprintf("   (Optical Media does not support Partitions)\n");
            return;
        }
        if(i->partitionCount == 0){
            kprintf("   (Drive not Partitioned)\n");
            return;
        }
        for(size_t j = 0; j < i->partitionCount; j++){
            kprintf("   Partition %d: Start LBA: 0x%x, End LBA: 0x%x, Size: %dKiB (%d Bytes)\n", j, i->partitions[j].startLBA, i->partitions[j].endLBA, i->partitions[j].size/1024, i->partitions[j].size);
        }
    }
}

static void console_user(){
    term_clear();
    exit = true;
}

static void console_shutdown(){
    power_shutdown();
}

static void console_reboot(){
    power_reboot();
}

void kcon_add_command(char* cmd, void* function){
    kcon_command_t* command = kmalloc(sizeof(kcon_command_t)); 
    memcpy(command->cmd, cmd, strlen(cmd));
    command->cmd[strlen(cmd)] = 0;
    command->function = function;

    list_insert(commandList, (void*)command); 
}

void init_kcon(){
    term_clear();

    commandList = list_create();

    kcon_add_command("help", console_help);
    kcon_add_command("clear", console_clear);
    kcon_add_command("ver", console_ver);
    kcon_add_command("mm", console_mm);
    kcon_add_command("timedate", console_timedate);
    kcon_add_command("dev", console_dev);
    kcon_add_command("cpu", console_cpu);
    kcon_add_command("fs", console_fs);
    kcon_add_command("drives", console_drives);
    kcon_add_command("user", console_user);
    kcon_add_command("shutdown", console_shutdown);
    kcon_add_command("reboot", console_reboot);

    kprintf("GarnOS Kernel Console Demo\n");
    char* cmd;

    kcon_command_t* command;

    while(true){
        if(exit) break;
        cmd = kreadline(">");
        if(cmd[0] == 0) continue;

        foreach(item, commandList){
            command = (kcon_command_t*)item->value;

            if(!strcmp(command->cmd, cmd)){
                command->function();
                goto foundCommand;
            }
        }

        kprintf("Unknown command: %s\n", cmd);
        continue;

foundCommand:
        continue;
    }
}
