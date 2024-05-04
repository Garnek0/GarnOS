/*  
*   File: kcon.c
*
*   Author: Garnek
*   
*   Description: Demo Kernel Console
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "kcon.h"
#include <kstdio.h>
#include <mem/pmm/pmm.h>
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>
#include <kernel.h>
#include <hw/rtc/rtc.h>
#include <display/fb.h>
#include <sys/term/term.h>
#include <sys/dal/dal.h>
#include <ds/list.h>
#include <sys/fal/fal.h>
#include <sys/power.h>

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
            "shutdown   - shutdown the system (non-ACPI)\n");
}

static void console_clear(){
    term_clear();
}

static void console_mm(){
    kprintf("available pages: %d (%uKiB free memory)\n"
            "used pages: %d (%uKiB used memory)\n"
            "kheap size: %uKiB\n", pmm_info.usablePages, (pmm_info.usablePages*PAGE_SIZE/1024), pmm_info.usedPages, (pmm_info.usedPages*PAGE_SIZE/1024), (kheap_get_size()/1024));
}

static void console_ver(){
    kprintf(KERNEL_VER"\n");
}

static char* weekdayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

static void console_timedate(){
    kprintf("%d/%d/%d %s\n", rtc.month, rtc.dayOfMonth, rtc.year, weekdayNames[rtc.weekday]);
    kprintf("%d:", rtc.hours);
    if(rtc.minutes < 10) kprintf("0");
    kprintf("%d:", rtc.minutes);
    if(rtc.seconds < 10) kprintf("0");
    kprintf("%d\n", rtc.seconds);
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
    filesys_t* filesys = filesys_get_all();
    for(size_t i = 0; i < MAX_FILESYSTEMS; i++){
        if(!filesys[i]._valid) continue;
        kprintf("%d: %s - Size: %dKiB (%d Bytes)\n", filesys[i].mountNumber, filesys[i].name, filesys[i].size/1024, filesys[i].size);
    }
}

static void console_drives(){
    drive_t* drives = drive_get_all();
    for(size_t i = 0; i < MAX_DRIVES; i++){
        if(!drives[i]._valid) continue;
        kprintf("%d) %s\n", i, drives[i].name);
        if(drives[i].type == DRIVE_TYPE_OPTICAL) {
            kprintf("   (Optical Media does not support Partitions)\n");
            return;
        }
        if(drives[i].partitionCount == 0){
            kprintf("   (Drive not Partitioned)\n");
            return;
        }
        for(int j = 0; j < drives[i].partitionCount; j++){
            kprintf("   Partition %d: Start LBA: 0x%x, End LBA: 0x%x, Size: %dKiB (%d Bytes)\n", j, drives[i].partitions[j].startLBA, drives[i].partitions[j].endLBA, drives[i].partitions[j].size/1024, drives[i].partitions[j].size);
        }
    }
}

static void console_user(){
    fb_clear(0x00000000);
    cursor_set(&tc.cursor, 0, 0);
    exit = true;
}

static void console_shutdown(){
    power_shutdown();
}

void kcon_add_command(char* cmd, void* function){
    kcon_command_t* command = kmalloc(sizeof(kcon_command_t)); 
    memcpy(command->cmd, cmd, strlen(cmd));
    command->cmd[strlen(cmd)] = 0;
    command->function = function;

    list_insert(commandList, (void*)command); 
}

void init_kcon(){
    fb_clear(0x00000000);
    cursor_set(&tc.cursor, 0, 0);

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