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
#include <mem/mm/pmm.h>
#include <mem/mm/kheap.h>
#include <mem/memutil/memutil.h>
#include <sys/rblogs.h>
#include <kernel.h>
#include <hw/rtc/rtc.h>
#include <display/fb.h>
#include <term/term.h>
#include <sys/device.h>
#include <ds/list.h>
#include <fs/vfs/vfs.h>

list_t* commandList;

static void console_help(){
    kprintf("commands:\n"
            "\n"
            "help       - show this text\n"
            "clear      - clear screen\n"
            "ver        - show kernel version\n"
            "mm         - show memory and mm info\n"
            "rblogs     - show logs inside the ringbuffer\n"
            "timedate   - show RTC Time and Date\n"
            "dev        - list all devices\n"
            "cpu        - show all detected processors\n"
            "fs         - list mounted filesystems\n"
            "drives     - list all drives\n"
            "halt       - halt the system\n");
}

static void console_clear(){
    term_clear();
}

static void console_mm(){
    kprintf("available pages: %d (%uKiB free memory)\n"
            "used pages: %d (%uKiB used memory)\n"
            "kheap size: %uKiB\n", pmm_info.usablePages, (pmm_info.usablePages*PAGE_SIZE/1024), pmm_info.usedPages, (pmm_info.usedPages*PAGE_SIZE/1024), (kheap_info.kheapSize/1024));
}

bool rblogsFirstIssued = true;
static void console_rblogs(){
    char* statusStrings[] = {
        "OK",
        "FAILED",
        "INFO",
        "WARNING",
        "FATAL"
    };

    if(rblogsFirstIssued){
        rblogsFirstIssued = false;
        kprintf("NOTE: ringbuffer logs are an experimental concept.\nThese logs may be incomplete or even inaccutare\n\n");
    }

    int spaces;
    for(int i = 0; i < RINGBUFFER_ENTRIES; i++){
        if(!RBEntries[i].log[0]) continue;
        spaces = 32;
        kprintf("LOG: %s", RBEntries[i].log);
        spaces -= strlen(RBEntries[i].log);
        for(int j = 0; j < spaces; j++) kprintf(" ");
        kprintf("STATUS: %s\n", statusStrings[RBEntries[i].status]);
    }
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
        if(!device.driver){
            kprintf("No Device Driver Loaded\n\n");
            continue;
        }
        kprintf("%s\n\n", device.driver->name);
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
        if(!filesys[i]._avail) continue;
        kprintf("%d: %s\n", filesys[i].mountNumber, filesys[i].name);
    }
}

static void console_drives(){
    drive_t* drives = drive_get_all();
    for(size_t i = 0; i < MAX_DRIVES; i++){
        if(!drives[i]._avail) continue;
        kprintf("%d) %s\n", i, drives[i].name);
    }
}

static void console_halt(){
    kprintf("Halted."); //only halts bsp
    asm volatile("cli");
    for(;;){
        asm volatile("hlt");
    }
}

void kcon_add_command(char* cmd, void* function){
    kcon_command_t* command = kmalloc(sizeof(kcon_command_t)); 
    memcpy(command->cmd, cmd, strlen(cmd));
    command->function = function;

    list_insert(commandList, (void*)command); 
}

void init_kcon(){
    fb_clear(0x00000000);
    cursor_set(&tc.cursor, 0, 0);

    commandList = list_create("commandList");

    kcon_add_command("help", console_help);
    kcon_add_command("clear", console_clear);
    kcon_add_command("ver", console_ver);
    kcon_add_command("mm", console_mm);
    kcon_add_command("rblogs", console_rblogs);
    kcon_add_command("timedate", console_timedate);
    kcon_add_command("dev", console_dev);
    kcon_add_command("cpu", console_cpu);
    kcon_add_command("fs", console_fs);
    kcon_add_command("drives", console_drives);
    kcon_add_command("halt", console_halt);

    kprintf("GarnOS Kernel Console Demo\n");
    char* cmd;

    kcon_command_t* command;

    while(true){
        cmd = kreadline(">");
        if(cmd[0] == 0) continue;

        foreach(item, commandList){
            command = (kcon_command_t*)item->value;

            if(!strcmp(command->cmd, cmd)){
                command->function();
                goto foundCommand;
            }
        }
        goto unknownCommand;

unknownCommand:
        kprintf("Unknown command: %s\n", cmd);
        continue;

foundCommand:
        continue;
    }
}