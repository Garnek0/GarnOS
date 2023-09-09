/*  
*   File: kcon.c
*
*   Author: Garnek
*   
*   Description: Demo Kernel Console
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <kstdio.h>
#include <mem/mm/pmm.h>
#include <mem/mm/kheap.h>
#include <mem/memutil/memutil.h>
#include <sys/rblogs.h>
#include <kernel.h>
#include <drivers/rtc/rtc.h>
#include <display/fb.h>
#include <term/term.h>

static void console_help(){
    kprintf("commands:\n"
            "\n"
            "help       - show this text\n"
            "ver        - show kernel version\n"
            "mm         - show memory and mm info\n"
            "rblogs     - show logs inside the ringbuffer\n"
            "timedate   - show RTC Time and Date\n");
}

static void console_mm(){
    kprintf("available pages: %d (%uKiB free memory)\n"
            "used pages: %d (%uKiB used memory)\n"
            "kheap size: %uKiB\n", usablePages, (usablePages*PAGE_SIZE/1024), usedPages, (usedPages*PAGE_SIZE/1024), (kheapSize/1024));
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

void init_kcon(){
    fb_clear(0x00000000);
    cursor_set(&tc.cursor, 0, 0);
    kprintf("GarnOS Kernel Console Demo\n");
    char* cmd;

    while(true){
        cmd = kreadline(">");
        if(!strcmp(cmd, "help")){
            console_help();
        } else if(!strcmp(cmd, "mm")){
            console_mm();
        } else if(!strcmp(cmd, "rblogs")){
            console_rblogs();
        } else if(!strcmp(cmd, "ver")){
            console_ver();
        } else if(!strcmp(cmd, "timedate")){
            console_timedate();
        } else if(cmd[0] == 0){
            continue;
        } else {
            kprintf("unknown command: %s\n", cmd);
        }
        kmfree(cmd);
    }
}