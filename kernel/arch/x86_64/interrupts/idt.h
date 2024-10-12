#pragma once

#include <garn/types.h>

#define INT_GATE 0x8E
#define INT_USER_GATE 0xEE
#define TRAP_GATE 0x8F

typedef struct {
    uint16_t size;
    uint64_t offset;
}__attribute__((packed)) idtr_t;

typedef struct {
    uint16_t offset0;
    uint16_t seg;
    uint8_t ist;
    uint8_t PDPLGateType;
    uint16_t offset1;
    uint32_t offset2;
    uint32_t reserved;
}__attribute__((packed)) idt_entry_t;

void idt_init();
void idt_set_entry(uint8_t entry, void* isr, uint8_t PDPLGateType);
