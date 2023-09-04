#include <kstdio.h>
#include <mem/mm/pmm.h>
#include <mem/mm/kheap.h>
#include <mem/memutil/memutil.h>

static void console_help(){
    kprintf("commands:\n"
            "\n"
            "help       - show this text\n"
            "mm         - show memory and mm info\n");
}

static void console_mm(){
    kprintf("available pages: %d (%uKiB free memory)\n"
            "used pages: %d (%uKiB used memory)\n"
            "kheap size: %uKiB\n", usablePages, (usablePages*PAGE_SIZE/1024), usedPages, (usedPages*PAGE_SIZE/1024), (kheapSize/1024));
}

void init_kcon(){
    kprintf("\nGarnOS Kernel Console Demo\n");
    char* cmd;

    while(true){
        cmd = kreadline(">");
        if(!strcmp(cmd, "help")){
            console_help();
        } else if(!strcmp(cmd, "mm")){
            console_mm();
        } else if(cmd[0] == 0){
            continue;
        } else {
            kprintf("unknown command: %s\n", cmd);
        }
        kmfree(cmd);
    }
}