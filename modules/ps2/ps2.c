#include <module/module.h>
#include <drivers/ports.h>
#include <cpu/interrupts/interrupts.h>
#include <kstdio.h>

#define PS2_DATA 0x60
#define PS2_COMMAND 0x64

void init(){

    asm volatile("cli");

    bool dualChannel = false;

    outb(PS2_COMMAND, 0xAD);
    outb(PS2_COMMAND, 0xA7);

    inb(PS2_DATA);

    outb(PS2_COMMAND, 0x20);            //get old config byte
    uint8_t configByte = inb(PS2_DATA); //
    configByte &= ~(1 | (1 << 1) | (1 << 6)); //clear some bits and set the new config byte
    outb(PS2_COMMAND, 0x60);                  //
    outb(PS2_DATA, configByte);               //

    outb(PS2_COMMAND, 0xAA);
    if(inb(PS2_DATA) != 0x55){
        klog("PS2: PS2 Self-test failed!\n", KLOG_WARNING);
        return;
    }
    outb(PS2_COMMAND, 0x60);
    outb(PS2_DATA, configByte);

    outb(PS2_COMMAND, 0xA8);
    outb(PS2_COMMAND, 0x20);
    configByte = inb(PS2_DATA);

    if(!(configByte & (1 << 5))){
        dualChannel = true;
        outb(PS2_COMMAND, 0xA7);
    }

    outb(PS2_COMMAND, 0xAB);
    if(inb(PS2_DATA)){
        klog("PS2: PS2 Port 1 faulty!\n", KLOG_WARNING);
    }

    if(dualChannel){
        outb(PS2_COMMAND, 0xA9);
        if(inb(PS2_DATA)){
            klog("PS2: PS2 Port 2 faulty!\n", KLOG_WARNING);
        }
    }

    outb(PS2_DATA, 0xFF);
    io_wait();
    inb(PS2_DATA);
    inb(PS2_DATA);
    inb(PS2_DATA);
    inb(PS2_DATA);

    outb(PS2_COMMAND, 0x20);
    configByte = inb(PS2_DATA);
    configByte |= (1 | (1 << 6));
    outb(PS2_COMMAND, 0x60);
    outb(PS2_DATA, configByte);

    if(dualChannel){
        outb(PS2_COMMAND, 0xD4);
        outb(PS2_DATA, 0xFF);
        io_wait();
        inb(PS2_DATA);
        inb(PS2_DATA);
        inb(PS2_DATA);
        inb(PS2_DATA);

        configByte |= (1 << 1);
        outb(PS2_COMMAND, 0x60);
        outb(PS2_DATA, configByte);
    }
    asm volatile("sti");

    klog("PS2: PS2 Controller Initialised.\n", KLOG_OK);
}

void fini(){
    return;
}

module_t metadata = {
    .name = "8042ps2",
    .init = init,
    .fini = fini
};